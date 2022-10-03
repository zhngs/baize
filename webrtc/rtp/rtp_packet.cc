#include "webrtc/rtp/rtp_packet.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

bool RtpPacket::IsRtp(StringPiece packet)
{
    // NOTE: RtcpPacket::IsRtcp() must always be called before this method.
    const uint8_t* data = packet.data_uint8();
    int len = packet.size();
    auto header = const_cast<Header*>(reinterpret_cast<const Header*>(data));

    return ((len >= kHeaderSize) &&
            // DOC:
            // https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
            (data[0] > 127 && data[0] < 192) &&
            // RTP Version must be 2.
            (header->version == 2));
}

RtpPacketUptr RtpPacket::Parse(StringPiece packet)
{
    const uint8_t* data = packet.data_uint8();
    int len = packet.size();

    if (!RtpPacket::IsRtp(packet)) return RtpPacketUptr();

    // parse header
    uint8_t* ptr = const_cast<uint8_t*>(data);
    Header* header = reinterpret_cast<Header*>(ptr);
    ptr += kHeaderSize;

    // parse csrc
    int csrc_list_size = 0;
    if (header->csrc_count != 0) {
        csrc_list_size =
            static_cast<int>(header->csrc_count * sizeof(header->ssrc));

        // Packet size must be >= header size + CSRC list.
        if (len < (ptr - data) + csrc_list_size) {
            LOG_ERROR << "not enough space for the announced CSRC list";
            return RtpPacketUptr();
        }
        ptr += csrc_list_size;
    }

    // parse extension
    HeaderExtension* header_extension = nullptr;
    int extension_value_size = 0;
    if (header->extension == 1) {
        // The header extension is at least 4 bytes.
        if (len < static_cast<int>(ptr - data) + 4) {
            LOG_ERROR << "not enough space for the announced header extension";
            return RtpPacketUptr();
        }
        header_extension = reinterpret_cast<HeaderExtension*>(ptr);

        // The header extension contains a 16-bit length field that counts the
        // number of 32-bit words in the extension, excluding the four-octet
        // header extension.
        extension_value_size =
            static_cast<int>(NetworkToHost16(header_extension->length) * 4);

        // Packet size must be >= header size + CSRC list + header extension
        // size.
        if (len < (ptr - data) + 4 + extension_value_size) {
            LOG_ERROR << "not enough space for the announced header extension";
            return RtpPacketUptr();
        }
        ptr += 4 + extension_value_size;
    }

    // parse payload and padding
    uint8_t* payload = ptr;
    int payload_length = static_cast<int>(len - (payload - data));
    uint8_t payload_padding = 0;
    if (header->padding != 0) {
        // Must be at least a single payload byte.
        if (payload_length == 0) {
            LOG_ERROR << "padding bit is set but no space for a padding byte";
            return RtpPacketUptr();
        }

        payload_padding = data[len - 1];
        if (payload_padding == 0) {
            LOG_ERROR << "padding byte cannot be 0, packet discarded";
            return RtpPacketUptr();
        }

        if (payload_length < payload_padding) {
            LOG_ERROR << "payload smaller than padding";
            return RtpPacketUptr();
        }

        payload_length -= payload_padding;
    }

    int packet_length = kHeaderSize + csrc_list_size +
                        (header_extension ? 4 + extension_value_size : 0) +
                        payload_length + payload_padding;
    if (packet_length != len) {
        return RtpPacketUptr();
    }

    auto rtp_packet = std::make_unique<RtpPacket>();
    rtp_packet->header_ = header;
    return rtp_packet;
}

RtpPacket::RtpPacket() {}

RtpPacket::~RtpPacket() {}

}  // namespace net

}  // namespace baize
