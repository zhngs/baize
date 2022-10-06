#include "webrtc/rtp/rtcp_packet.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

bool RtcpPacket::IsRtcp(StringPiece packet)
{
    const uint8_t* data = packet.data_uint8();
    int len = packet.size();

    auto header =
        const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

    return (
        (len >= kCommonHeaderSize) &&
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

string RtcpPacket::Dump(const CommonHeader& header)
{
    string s;
    log::StringAppend(s, "    version(2): %u", header.version);
    log::StringAppend(s, "    padding(1): %u", header.padding);
    log::StringAppend(s, "    count(5): %u", header.count);
    log::StringAppend(s, "    type(8): %u", header.type);
    log::StringAppend(s, "    length(16): %u", header.length);
    return s;
}

string RtcpFeedback::Dump(const Header& header)
{
    string s;
    log::StringAppend(s, "    sender_ssrc: %u", header.sender_ssrc);
    log::StringAppend(s, "    media_ssrc: %u", header.media_ssrc);
    return s;
}

int PliPsFb::Decode(StringPiece packet)
{
    uint8_t* data = packet.data_uint8();
    int len = packet.size();

    if (len != RtcpPacket::kCommonHeaderSize + RtcpFeedback::kHeaderSize) {
        LOG_ERROR << "err len for Pli";
        return -1;
    }

    RtcpPacket::CommonHeader* common_header =
        reinterpret_cast<RtcpPacket::CommonHeader*>(data);
    this->comm_header = *common_header;
    this->comm_header.length = NetworkToHost16(this->comm_header.length);

    RtcpFeedback::Header* feedback_header =
        reinterpret_cast<RtcpFeedback::Header*>(data +
                                                RtcpPacket::kCommonHeaderSize);
    this->fb_header = *feedback_header;
    this->fb_header.sender_ssrc = NetworkToHost32(this->fb_header.sender_ssrc);
    this->fb_header.media_ssrc = NetworkToHost32(this->fb_header.media_ssrc);

    return 0;
}

int PliPsFb::Encode(StringPiece& packet)
{
    uint8_t* data = packet.data_uint8();
    int len = packet.size();

    int encode_len = RtcpPacket::kCommonHeaderSize + RtcpFeedback::kHeaderSize;
    if (len < encode_len) {
        LOG_ERROR << "space too short";
        return -1;
    }

    auto tmp_comm_header = this->comm_header;
    tmp_comm_header.length = HostToNetwork16(tmp_comm_header.length);
    memcpy(data, &tmp_comm_header, sizeof(tmp_comm_header));

    auto tmp_fb_header = this->fb_header;
    tmp_fb_header.sender_ssrc = HostToNetwork32(tmp_fb_header.sender_ssrc);
    tmp_fb_header.media_ssrc = HostToNetwork32(tmp_fb_header.media_ssrc);
    memcpy(data + RtcpPacket::kCommonHeaderSize,
           &tmp_fb_header,
           sizeof(tmp_fb_header));

    packet.set(data, encode_len);

    return 0;
}

string PliPsFb::Dump()
{
    string s;
    log::StringAppend(s, "\n<PliPsFb>");
    s += RtcpPacket::Dump(this->comm_header);
    s += RtcpFeedback::Dump(this->fb_header);
    log::StringAppend(s, "</PliPsFb>");
    return s;
}

string RtcpReport::Dump(const Report& item)
{
    string s;
    log::StringAppend(s, "  <block>");
    log::StringAppend(s, "    fraction lost: %u", item.fraction_lost);
    log::StringAppend(s, "    total lost: %u", item.total_lost);
    log::StringAppend(s, "    last seq: %u", item.last_seq);
    log::StringAppend(s, "    jitter: %u", item.jitter);
    log::StringAppend(s, "    lsr: %u", item.lsr);
    log::StringAppend(s, "    dlsr: %u", item.dlsr);
    log::StringAppend(s, "  </block>");
    return s;
}

int RrPacket::Decode(StringPiece packet)
{
    uint8_t* data = packet.data_uint8();
    int len = packet.size();

    if (len < RtcpPacket::kCommonHeaderSize + 4) {
        LOG_ERROR << "err len for RR";
        return -1;
    }

    RtcpPacket::CommonHeader* common_header =
        reinterpret_cast<RtcpPacket::CommonHeader*>(data);
    this->comm_header = *common_header;
    this->comm_header.length = NetworkToHost16(this->comm_header.length);

    this->sender_ssrc = byte4(data, RtcpPacket::kCommonHeaderSize);

    uint8_t count = common_header->count;
    int offset = RtcpPacket::kCommonHeaderSize + 4;
    while ((count--) && (len > offset)) {
        if (len - offset < RtcpReport::kReportSize) {
            LOG_ERROR << "space too short for rr report";
            break;
        }
        RtcpReport::Report* report =
            reinterpret_cast<RtcpReport::Report*>(data + offset);
        RtcpReport::Report item = *report;

        item.ssrc = NetworkToHost32(item.ssrc);
        item.last_seq = NetworkToHost32(item.last_seq);
        item.jitter = NetworkToHost32(item.jitter);
        item.lsr = NetworkToHost32(item.lsr);
        item.dlsr = NetworkToHost32(item.dlsr);

        // todo: may be error byte order
        item.total_lost = byte3(&item, 5);

        reports.push_back(item);
        offset += RtcpReport::kReportSize;
    }

    return 0;
}

string RrPacket::Dump()
{
    string s;
    log::StringAppend(s, "\n<ReceiverReport>");
    s += RtcpPacket::Dump(this->comm_header);
    log::StringAppend(s, "    sender_ssrc: %u", sender_ssrc);
    for (auto& item : reports) {
        s += RtcpReport::Dump(item);
    }
    log::StringAppend(s, "</ReceiverReport>");

    return s;
}

int SrPacket::Decode(StringPiece packet)
{
    uint8_t* data = packet.data_uint8();
    int len = packet.size();

    if (len < RtcpPacket::kCommonHeaderSize + kSendInfoSize) {
        LOG_ERROR << "err len for SR";
        return -1;
    }

    RtcpPacket::CommonHeader* common_header =
        reinterpret_cast<RtcpPacket::CommonHeader*>(data);
    this->comm_header = *common_header;
    this->comm_header.length = NetworkToHost16(this->comm_header.length);

    SendInfo* psend_info =
        reinterpret_cast<SendInfo*>(data + RtcpPacket::kCommonHeaderSize);
    this->send_info = *psend_info;
    this->send_info.ssrc = NetworkToHost32(this->send_info.ssrc);
    this->send_info.ntp_sec = NetworkToHost32(this->send_info.ntp_sec);
    this->send_info.ntp_frac = NetworkToHost32(this->send_info.ntp_frac);
    this->send_info.rtp_ts = NetworkToHost32(this->send_info.rtp_ts);
    this->send_info.packet_count =
        NetworkToHost32(this->send_info.packet_count);
    this->send_info.octet_count = NetworkToHost32(this->send_info.octet_count);

    uint8_t count = common_header->count;
    int offset = RtcpPacket::kCommonHeaderSize + kSendInfoSize;
    while ((count--) && (len > offset)) {
        if (len - offset < RtcpReport::kReportSize) {
            LOG_ERROR << "space too short for rr report";
            break;
        }
        RtcpReport::Report* report =
            reinterpret_cast<RtcpReport::Report*>(data + offset);
        RtcpReport::Report item = *report;

        item.ssrc = NetworkToHost32(item.ssrc);
        item.last_seq = NetworkToHost32(item.last_seq);
        item.jitter = NetworkToHost32(item.jitter);
        item.lsr = NetworkToHost32(item.lsr);
        item.dlsr = NetworkToHost32(item.dlsr);

        // todo: may be error byte order
        item.total_lost = byte3(&item, 5);

        reports.push_back(item);
        offset += RtcpReport::kReportSize;
    }

    return 0;
}

string SrPacket::Dump()
{
    string s;
    log::StringAppend(s, "\n<SenderReport>");
    s += RtcpPacket::Dump(this->comm_header);
    log::StringAppend(s, "    sender_ssrc: %u", this->send_info.ssrc);
    log::StringAppend(s, "    ntp_sec: %u", this->send_info.ntp_sec);
    log::StringAppend(s, "    ntp_frac: %u", this->send_info.ntp_frac);
    log::StringAppend(s, "    rtp_ts: %u", this->send_info.rtp_ts);
    log::StringAppend(s, "    packet_count: %u", this->send_info.packet_count);
    log::StringAppend(s, "    octet_count: %u", this->send_info.octet_count);
    for (auto& item : reports) {
        s += RtcpReport::Dump(item);
    }
    log::StringAppend(s, "</SenderReport>");

    return s;
}

int SdesPacket::Decode(StringPiece packet)
{
    uint8_t* data = packet.data_uint8();
    int len = packet.size();

    if (len < RtcpPacket::kCommonHeaderSize) {
        LOG_ERROR << "err len for Sdes";
        return -1;
    }

    RtcpPacket::CommonHeader* common_header =
        reinterpret_cast<RtcpPacket::CommonHeader*>(data);
    this->comm_header = *common_header;
    this->comm_header.length = NetworkToHost16(this->comm_header.length);

    int offset = RtcpPacket::kCommonHeaderSize;
    uint8_t count = common_header->count;
    while ((count--) && (len > offset)) {
        if (len - offset < 4) {
            LOG_ERROR << "no space for sdes chunk";
            break;
        }

        Chunk chunk;
        chunk.ssrc = byte4(data, offset);
        chunk.items = DecodeItem(StringPiece(data + offset, len - offset));

        chunks.push_back(chunk);

        offset += chunk.size();
    }

    return 0;
}

std::vector<SdesPacket::Item> SdesPacket::DecodeItem(StringPiece packet)
{
    uint8_t* data = packet.data_uint8();
    int len = packet.size();
    std::vector<Item> items_vector;

    int offset = 0;
    while (len > offset) {
        int left_len = len - offset;
        if (left_len < 2 || left_len < 2 + data[1]) {
            LOG_ERROR << "no space for sdes item";
            break;
        }

        Item item;
        item.type = static_cast<Type>(data[offset]);
        item.length = data[offset + 1];
        item.value.assign(reinterpret_cast<char*>(offset + 2), item.length);

        if (item.type == Type::END) {
            break;
        }

        items_vector.push_back(item);
        offset += item.size();
    }

    return items_vector;
}

string SdesPacket::Dump()
{
    string s;
    log::StringAppend(s, "\n<Sdes>");
    s += RtcpPacket::Dump(this->comm_header);
    for (auto& chunk : chunks) {
        log::StringAppend(s, "  <chunk>");
        log::StringAppend(s, "  ssrc: %u", chunk.ssrc);
        for (auto& item : chunk.items) {
            log::StringAppend(s, "    <item>");
            log::StringAppend(s, "    type: %u", item.type);
            log::StringAppend(s, "    length: %u", item.length);
            log::StringAppend(s, "    value: %s", item.value.c_str());
            log::StringAppend(s, "    </item>");
        }
        log::StringAppend(s, "  </chunk>");
    }
    log::StringAppend(s, "</Sdes>");

    return s;
}

}  // namespace net

}  // namespace baize
