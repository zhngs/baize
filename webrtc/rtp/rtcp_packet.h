#ifndef BAIZE_RTCP_PACKET_H_
#define BAIZE_RTCP_PACKET_H_

#include <memory>
#include <vector>

#include "net/net_buffer.h"
#include "util/string_piece.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class RtcpPacket  // copyable
{
public:  // types
    static const int kCommonHeaderSize = 4;

    enum class Type : uint8_t {
        SR = 200,
        RR = 201,
        SDES = 202,
        BYE = 203,
        APP = 204,
        RTPFB = 205,
        PSFB = 206,
        XR = 207
    };

#pragma pack(1)
    struct CommonHeader {
#if defined(BAIZE_BIG_ENDIAN)
        uint8_t version : 2;
        uint8_t padding : 1;
        uint8_t count : 5;
#elif defined(BAIZE_LITTLE_ENDIAN)
        uint8_t count : 5;
        uint8_t padding : 1;
        uint8_t version : 2;
#endif
        uint8_t type;
        uint16_t length;

        CommonHeader() { MemoryZero(this, sizeof(*this)); }
    };
#pragma pack()

public:
    static bool IsRtcp(StringPiece packet);
    static string Dump(const CommonHeader& header);
};

class RtcpFeedback
{
public:  // types
    static const int kHeaderSize = 8;

    enum class PsType : uint8_t {
        PLI = 1,
        SLI = 2,
        RPSI = 3,
        FIR = 4,
        TSTR = 5,
        TSTN = 6,
        VBCM = 7,
        PSLEI = 8,
        ROI = 9,
        AFB = 15,
        EXT = 31
    };

    enum class RtpType : uint8_t {
        NACK = 1,
        TMMBR = 3,
        TMMBN = 4,
        SR_REQ = 5,
        RAMS = 6,
        TLLEI = 7,
        ECN = 8,
        PS = 9,
        TCC = 15,
        EXT = 31
    };

#pragma pack(1)
    struct Header {
        uint32_t sender_ssrc;
        uint32_t media_ssrc;

        Header() { MemoryZero(this, sizeof(*this)); }
    };
#pragma pack()

public:
    static string Dump(const Header& header);
};

class PliPsFb  // copyable
{
public:
    int Decode(StringPiece packet);
    int Encode(StringPiece& packet);

    string Dump();

public:
    RtcpPacket::CommonHeader comm_header;
    RtcpFeedback::Header fb_header;
};

class RtcpReport
{
public:
    static const int kReportSize = 24;
#pragma pack(1)
    struct Report {
        uint32_t ssrc;
        uint32_t fraction_lost : 8;
        uint32_t total_lost : 24;
        uint32_t last_seq;
        uint32_t jitter;
        uint32_t lsr;
        uint32_t dlsr;

        Report() { MemoryZero(this, sizeof(*this)); }
    };
#pragma pack()
public:
    static string Dump(const Report& report);
};

class RrPacket
{
public:
    int Decode(StringPiece packet);
    int Encode(StringPiece& packet);

    string Dump();

public:
    RtcpPacket::CommonHeader comm_header;
    uint32_t sender_ssrc = 0;
    std::vector<RtcpReport::Report> reports;
};

class SrPacket
{
public:
    static const int kSendInfoSize = 24;
#pragma pack(1)
    struct SendInfo {
        uint32_t ssrc;
        uint32_t ntp_sec;
        uint32_t ntp_frac;
        uint32_t rtp_ts;
        uint32_t packet_count;
        uint32_t octet_count;

        SendInfo() { MemoryZero(this, sizeof(*this)); }
    };
#pragma pack(1)

public:
    int Decode(StringPiece packet);
    int Encode(StringPiece& packet);

    string Dump();

public:
    RtcpPacket::CommonHeader comm_header;
    SendInfo send_info;
    std::vector<RtcpReport::Report> reports;
};

class SdesPacket
{
public:
    enum class Type : uint8_t {
        END = 0,
        CNAME,
        NAME,
        EMAIL,
        PHONE,
        LOC,
        TOOL,
        NOTE,
        PRIV
    };

    struct Item {
        Type type;
        uint8_t length;
        string value;

        Item() : type(Type::END), length(0) {}
        int size() { return 2 + length; }
    };
    struct Chunk {
        uint32_t ssrc;
        std::vector<Item> items;

        Chunk() : ssrc(0) {}
        int size()
        {
            int ret = 4;
            for (auto& item : items) {
                ret += item.size();
            }
            return ret;
        }
    };

public:
    int Decode(StringPiece packet);
    int Encode(StringPiece& packet);

    string Dump();

private:
    std::vector<Item> DecodeItem(StringPiece packet);

public:
    RtcpPacket::CommonHeader comm_header;
    std::vector<Chunk> chunks;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_RTCP_PACKET_H_