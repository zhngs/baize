#include "webrtc/pc/peer_connection.h"

#include "log/logger.h"
#include "webrtc/pc/webrtc_server.h"
#include "webrtc/pc/webrtc_settings.h"

namespace baize
{

namespace net
{

PeerConnection::PeerConnection(UdpStreamSptr stream,
                               InetAddress addr,
                               void* arg)
  : stream_(stream), addr_(addr), ext_arg_(arg)
{
}

PeerConnection::~PeerConnection() {}

PeerConnection::Packet PeerConnection::AsyncRead()
{
    while (1) {
        if (packets_.empty()) {
            async_park_.WaitRead();
            continue;
        }
        Packet packet = std::move(packets_.back());
        packets_.pop_back();
        return std::move(packet);
    }
}

int PeerConnection::AsyncWrite(StringPiece packet)
{
    return stream_->AsyncSendto(packet.data(), packet.size(), addr_);
}

int PeerConnection::ProcessPacket(StringPiece packet)
{
    /**
     * handle ice
     */

    if (!ice_) {
        ice_ = IceServer::New(WebRTCSettings::ice_password(), stream_, addr_);
    }

    if (StunPacket::IsStun(packet)) {
        LOG_INFO << "process stun";
        ice_->ProcessStunPacket(packet);
        return 0;
    }

    if (!ice_->is_connected()) {
        return 0;
    }

    /**
     * handle dtls
     */

    if (!dtls_) {
        dtls_ = DtlsTransport::New(WebRTCSettings::dtls_ctx(), stream_, addr_);
        dtls_->Initialize(DtlsTransport::Role::SERVER);
    }

    if (DtlsTransport::IsDtls(packet)) {
        LOG_INFO << "process dtls";
        dtls_->ProcessDtlsPacket(packet);
        return 0;
    }

    if (!dtls_->is_running()) {
        return -1;
    } else if (!dtls_->is_connected()) {
        LOG_INFO << "dtls not connected";
        return 0;
    }

    /**
     * handle rtp and rtcp
     */
    if (!srtp_send_session_) {
        srtp_send_session_ = SrtpSession::New(SrtpSession::Type::kOutBound,
                                              dtls_->use_srtp(),
                                              dtls_->srtp_local_master());
        if (!srtp_send_session_) {
            LOG_ERROR << "srtp_send_session err";
            return -1;
        }
    }

    if (!srtp_recv_session_) {
        srtp_recv_session_ = SrtpSession::New(SrtpSession::Type::kInBound,
                                              dtls_->use_srtp(),
                                              dtls_->srtp_remote_master());
        if (!srtp_recv_session_) {
            LOG_ERROR << "srtp_recv_session err";
            return -1;
        }
    }

    static int only_once = 0;
    only_once++;
    if (only_once % 30 == 0) {
        PliPsFb pli;
        pli.comm_header.version = 2;
        pli.comm_header.padding = 0;
        pli.comm_header.count = 1;
        pli.comm_header.type = 206;
        pli.comm_header.length = 2;
        pli.fb_header.sender_ssrc = 1234;
        string media_ssrc = current_pub_sdp().tracks_[0].ssrc_group_[0];
        pli.fb_header.media_ssrc = atoi(media_ssrc.c_str());

        LOG_INFO << "send to peer" << pli.Dump();

        char buf[1500] = "";
        StringPiece pli_packet(buf, 1500);
        pli.Encode(pli_packet);

        srtp_send_session_->EncryptRtcp(pli_packet);
        AsyncWrite(pli_packet);

        // only_once = false;
    }

    if (RtcpPacket::IsRtcp(packet)) {
        bool err = srtp_recv_session_->DecryptRtcp(packet);
        if (!err) {
            LOG_ERROR << "decrypt rtcp err";
            return -1;
        }
        return ProcessRtcp(packet);
    }

    if (RtpPacket::IsRtp(packet)) {
        bool err = srtp_recv_session_->DecryptRtp(packet);
        if (!err) {
            LOG_ERROR << "decrypt rtcp err";
            return -1;
        }

        RtpPacket rtp;
        int val = rtp.Decode(packet);
        if (val < 0) {
            LOG_ERROR << "handle rtp err";
            return 0;
        }
        LOG_INFO << rtp.Dump();

        RtpPacket::Header* dummy_header =
            reinterpret_cast<RtpPacket::Header*>(packet.data());
        dummy_header->ssrc = HostToNetwork32(1234);

        err = srtp_send_session_->EncryptRtp(packet);
        if (!err) {
            LOG_ERROR << "decrypt rtcp err";
            return -1;
        }
        AsyncWrite(packet);

        // WebRTCServer* server = reinterpret_cast<WebRTCServer*>(ext_arg_);
        // auto room = server->room();
        // string selfkey = addr_.ip_port();
        // for (auto& item : room) {
        //     if (item.first == selfkey) continue;
        //     auto pc_sptr = item.second.lock();
        //     if (pc_sptr && pc_sptr->srtp_send_session_) {
        //         err = pc_sptr->srtp_send_session_->EncryptRtp(packet);
        //         if (!err) {
        //             LOG_ERROR << "decrypt rtcp err";
        //             return -1;
        //         }
        //         pc_sptr->AsyncWrite(packet);
        //     }
        // }

        return 0;
    }

    return 0;
}

int PeerConnection::ProcessRtcp(StringPiece packet)
{
    uint8_t* data = packet.data_uint8();
    int len = packet.size();

    while (len > 0) {
        if (!RtcpPacket::IsRtcp(packet)) {
            LOG_ERROR << "data is not a RTCP packet";
            return -1;
        }

        RtcpPacket::CommonHeader* header =
            reinterpret_cast<RtcpPacket::CommonHeader*>(data);
        int packet_len =
            static_cast<int>(NetworkToHost16(header->length) + 1) * 4;

        if (len < packet_len) {
            LOG_ERROR << "rtcp packet len error";
            return -1;
        }

        switch (RtcpPacket::Type(header->type)) {
            case RtcpPacket::Type::SR: {
                LOG_INFO << "RTCP packet type: SR";
                SrPacket sr;
                sr.Decode(packet);
                LOG_INFO << sr.Dump();

                break;
            }
            case RtcpPacket::Type::RR: {
                LOG_INFO << "RTCP packet type: RR";
                RrPacket rr;
                rr.Decode(packet);
                LOG_INFO << rr.Dump();
                break;
            }
            case RtcpPacket::Type::SDES: {
                LOG_INFO << "RTCP packet type: SDES";
                SdesPacket sdes;
                sdes.Decode(packet);
                LOG_INFO << sdes.Dump();
                break;
            }
            case RtcpPacket::Type::BYE: {
                LOG_INFO << "RTCP packet type: BYE";
                break;
            }
            case RtcpPacket::Type::APP: {
                LOG_INFO << "RTCP packet type: APP";
                break;
            }
            case RtcpPacket::Type::RTPFB: {
                LOG_INFO << "RTCP packet type: RTPFB";
                break;
            }
            case RtcpPacket::Type::PSFB: {
                LOG_INFO << "RTCP packet type: PSFB";

                switch (RtcpFeedback::PsType(header->count)) {
                    case RtcpFeedback::PsType::PLI: {
                        LOG_INFO << "RTCP PS Feedback PLI received";
                        PliPsFb pli;
                        pli.Decode(packet);
                        LOG_INFO << pli.Dump();
                        break;
                    }
                    default: {
                        LOG_ERROR << "unknown RTCP PS Feedback message type "
                                  << header->count;
                    }
                }

                break;
            }
            case RtcpPacket::Type::XR: {
                LOG_INFO << "RTCP packet type: XR";
                break;
            }
            default: {
                LOG_ERROR << "unknown RTCP packet type: " << header->type;
            }
        }

        data += packet_len;
        len -= packet_len;
    }

    return 0;
}

}  // namespace net

}  // namespace baize