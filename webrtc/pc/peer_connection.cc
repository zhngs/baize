#include "webrtc/pc/peer_connection.h"

#include "log/logger.h"
#include "webrtc/pc/webrtc_settings.h"

namespace baize
{

namespace net
{

PeerConnection::PeerConnection(UdpStreamSptr stream, InetAddress addr)
  : stream_(stream), addr_(addr)
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

int PeerConnection::ProcessPacket(StringPiece packet)
{
    /**
     * handle ice
     */
    if (StunPacket::IsStun(packet)) {
        if (!ice_) {
            ice_ =
                IceServer::New(WebRTCSettings::ice_password(), stream_, addr_);
        }

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

    if (DtlsTransport::IsDtls(packet)) {
        if (!dtls_) {
            dtls_ =
                DtlsTransport::New(WebRTCSettings::dtls_ctx(), stream_, addr_);
            dtls_->Initialize(DtlsTransport::Role::SERVER);
        }

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

    if (RtcpPacket::IsRtcp(packet)) {
        bool err = srtp_recv_session_->DecryptRtcp(packet);
        if (!err) {
            LOG_ERROR << "decrypt rtcp err";
            return -1;
        }
        auto rtcp_group = RtcpPacket::Parse(packet);
        LOG_INFO << "rtcp group: " << rtcp_group.size();

        return 0;
    }

    if (RtpPacket::IsRtp(packet)) {
        bool err = srtp_recv_session_->DecryptRtp(packet);
        if (!err) {
            LOG_ERROR << "decrypt rtcp err";
            return -1;
        }

        RtpPacketUptr rtp_packet = RtpPacket::Parse(packet);
        if (!rtp_packet) {
            LOG_ERROR << "handle rtp err";
            return 0;
        }

        LOG_INFO << "handle rtp";
        // 黑魔法
        auto header = rtp_packet->header();
        header->ssrc = HostToNetwork32(1234);

        err = srtp_send_session_->EncryptRtp(packet);
        if (!err) {
            LOG_ERROR << "decrypt rtcp err";
            return -1;
        }
        stream_->AsyncSendto(packet.data(), packet.size(), addr_);

        return 0;
    }

    return 0;
}

}  // namespace net

}  // namespace baize