#include "webrtc/rtp/rtcp_packet.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

RtcpPacket::RtcpPacket() {}

RtcpPacket::~RtcpPacket() {}

bool RtcpPacket::IsRtcp(StringPiece packet)
{
    const uint8_t* data = packet.data_uint8();
    int len = packet.size();

    auto header =
        const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

    return (
        (len >= kHeaderSize) &&
        // DOC:
        // https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
        (data[0] > 127 && data[0] < 192) &&
        // RTP Version must be 2.
        (header->version == 2) &&
        // RTCP packet types defined by IANA:
        // http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml#rtp-parameters-4
        // RFC 5761 (RTCP-mux) states this range for secure RTCP/RTP
        // detection.
        (header->type >= 192 && header->type <= 223));
}

RtcpPacketGroup RtcpPacket::Parse(StringPiece packet)
{
    const uint8_t* data = packet.data_uint8();
    int len = packet.size();

    RtcpPacketGroup rtcp_group;
    RtcpPacketUptr current;

    while (len > 0) {
        if (!RtcpPacket::IsRtcp(packet)) {
            LOG_ERROR << "data is not a RTCP packet";
            return rtcp_group;
        }

        auto* header = const_cast<CommonHeader*>(
            reinterpret_cast<const CommonHeader*>(data));
        int packet_len =
            static_cast<int>(NetworkToHost16(header->length) + 1) * 4;

        if (len < packet_len) {
            LOG_ERROR << "rtcp packet len error";
            return rtcp_group;
        }

        LOG_INFO << "RTCP packet type: " << header->type;
        switch (Type(header->type)) {
            case Type::SR: {
                // current = SenderReportPacket::Parse(data, packet_len);
                // if (!current) break;

                // if (header->count > 0) {
                //     Packet* rr = ReceiverReportPacket::Parse(
                //         data, packetLen, current->GetSize());
                //     if (!rr) break;
                //     current->SetNext(rr);
                // }

                break;
            }
            case Type::RR: {
                // current = ReceiverReportPacket::Parse(data, packet_len);
                break;
            }
            case Type::SDES: {
                // current = SdesPacket::Parse(data, packet_len);
                break;
            }
            case Type::BYE: {
                // current = ByePacket::Parse(data, packet_len);
                break;
            }
            case Type::APP: {
                current.reset();
                break;
            }
            case Type::RTPFB: {
                // current = FeedbackRtpPacket::Parse(data, packet_len);
                break;
            }
            case Type::PSFB: {
                // current = FeedbackPsPacket::Parse(data, packet_len);
                break;
            }
            case Type::XR: {
                // current = ExtendedReportPacket::Parse(data, packet_len);
                break;
            }
            default: {
                LOG_ERROR << "unknown RTCP packet type: " << header->type;
                current.reset();
            }
        }

        if (!current) {
            return rtcp_group;
        }

        data += packet_len;
        len -= packet_len;

        rtcp_group.emplace_back(std::move(current));
    }

    return rtcp_group;
}

}  // namespace net

}  // namespace baize
