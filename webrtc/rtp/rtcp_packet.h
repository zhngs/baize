#ifndef BAIZE_RTCP_PACKET_H_
#define BAIZE_RTCP_PACKET_H_

#include <memory>
#include <vector>

#include "util/string_piece.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class RtcpPacket;
using RtcpPacketUptr = std::unique_ptr<RtcpPacket>;
using RtcpPacketGroup = std::vector<RtcpPacketUptr>;

class RtcpPacket  // copyable
{
public:  // types
    static const int kHeaderSize = 4;

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
    };
#pragma pack()

public:  // static method and factory method
    static bool IsRtcp(StringPiece packet);
    static RtcpPacketGroup Parse(StringPiece packet);

public:
    RtcpPacket();
    ~RtcpPacket();

private:
    CommonHeader head_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_RTCP_PACKET_H_