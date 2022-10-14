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

int RtpPacket::Decode(StringPiece packet)
{
    const uint8_t* data = packet.data_uint8();
    int len = packet.size();

    if (!RtpPacket::IsRtp(packet)) return -1;

    if (len < kHeaderSize) return -1;

    // parse header
    uint8_t* ptr = const_cast<uint8_t*>(data);
    Header* header = reinterpret_cast<Header*>(ptr);

    this->rtp_header = *header;
    this->rtp_header.sequence_number =
        NetworkToHost16(this->rtp_header.sequence_number);
    this->rtp_header.timestamp = NetworkToHost32(this->rtp_header.timestamp);
    this->rtp_header.ssrc = NetworkToHost32(this->rtp_header.ssrc);

    ptr += kHeaderSize;

    // parse csrc
    int csrc_list_size = 0;
    if (header->csrc_count != 0) {
        csrc_list_size =
            static_cast<int>(header->csrc_count * sizeof(header->ssrc));

        // Packet size must be >= header size + CSRC list.
        if (len < (ptr - data) + csrc_list_size) {
            LOG_ERROR << "not enough space for the announced CSRC list";
            return -1;
        }

        uint8_t count = header->csrc_count;
        int offset = static_cast<int>(ptr - data);
        while (count--) {
            this->csrcs.push_back(byte4(data, offset));
            offset += 4;
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
            return -1;
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
            return -1;
        }

        this->extension.header = *header_extension;
        this->extension.header.profile =
            NetworkToHost16(this->extension.header.profile);
        this->extension.header.length =
            NetworkToHost16(this->extension.header.length);

        ptr += 4 + extension_value_size;
    }

    // parse payload and padding
    uint8_t* rtp_payload = ptr;
    int payload_length = static_cast<int>(len - (rtp_payload - data));
    uint8_t payload_padding = 0;
    if (header->padding != 0) {
        // Must be at least a single payload byte.
        if (payload_length == 0) {
            LOG_ERROR << "padding bit is set but no space for a padding byte";
            return -1;
        }

        payload_padding = data[len - 1];
        if (payload_padding == 0) {
            LOG_ERROR << "padding byte cannot be 0, packet discarded";
            return -1;
        }

        if (payload_length < payload_padding) {
            LOG_ERROR << "payload smaller than padding";
            return -1;
        }

        payload_length -= payload_padding;
    }

    this->payload.assign(reinterpret_cast<char*>(rtp_payload), payload_length);

    int packet_length = kHeaderSize + csrc_list_size +
                        (header_extension ? 4 + extension_value_size : 0) +
                        payload_length + payload_padding;
    if (packet_length != len) {
        return -1;
    }

    return 0;
}

string RtpPacket::Dump()
{
    string s;
    log::StringAppend(s, "\n<Rtp>");
    log::StringAppend(s, "    version: %u", this->rtp_header.version);
    log::StringAppend(s, "    pading: %u", this->rtp_header.padding);
    log::StringAppend(s, "    extension: %u", this->rtp_header.extension);
    log::StringAppend(s, "    csrc_count: %u", this->rtp_header.csrc_count);
    log::StringAppend(s, "    marker: %u", this->rtp_header.marker);
    log::StringAppend(s, "    payload_type: %u", this->rtp_header.payload_type);
    log::StringAppend(s, "    seq: %u", this->rtp_header.sequence_number);
    log::StringAppend(s, "    timestamp: %u", this->rtp_header.timestamp);
    log::StringAppend(s, "    ssrc: %u", this->rtp_header.ssrc);
    if (this->rtp_header.extension) {
        log::StringAppend(s, "  <Extension>");
        log::StringAppend(
            s, "    profile: %#x", this->extension.header.profile);
        log::StringAppend(s, "    length: %u", this->extension.header.length);
        log::StringAppend(s, "  </Extension>");
    }
    log::StringAppend(s, "    payload_length: %u", this->payload.length());
    log::StringAppend(s, "</Rtp>");
    return s;
}

}  // namespace net

}  // namespace baize
