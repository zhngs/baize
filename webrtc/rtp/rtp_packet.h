#ifndef BAIZE_RTP_PACKET_H_
#define BAIZE_RTP_PACKET_H_

#include <memory>

#include "util/string_piece.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class RtpPacket;
using RtpPacketUptr = std::unique_ptr<RtpPacket>;

class RtpPacket
{
public:  // type
#pragma pack(1)
    struct Header {
#if defined(BAIZE_LITTLE_ENDIAN)
        uint8_t csrc_count : 4;
        uint8_t extension : 1;
        uint8_t padding : 1;
        uint8_t version : 2;
        uint8_t payload_type : 7;
        uint8_t marker : 1;
#elif defined(BAIZE_BIG_ENDIAN)
        uint8_t version : 2;
        uint8_t padding : 1;
        uint8_t extension : 1;
        uint8_t csrc_count : 4;
        uint8_t marker : 1;
        uint8_t payload_type : 7;
#endif
        uint16_t sequence_number;
        uint32_t timestamp;
        uint32_t ssrc;
    };

    struct HeaderExtension {
        uint16_t profile;
        uint16_t length;
        uint8_t value[0];
    };

    struct OneByteExtension {
#if defined(BAIZE_LITTLE_ENDIAN)
        uint8_t len : 4;
        uint8_t id : 4;
#elif defined(BAIZE_BIG_ENDIAN)
        uint8_t id : 4;
        uint8_t len : 4;
#endif
        uint8_t value[0];
    };

    struct TwoBytesExtension {
        uint8_t id;
        uint8_t len;
        uint8_t value[0];
    };
#pragma pack()

public:  // static method
    static const int kHeaderSize = 12;
    static bool IsRtp(StringPiece packet);

    static RtpPacketUptr Parse(StringPiece packet);

public:
    RtpPacket();
    ~RtpPacket();

public:  // getter and setter
    Header* header() { return header_; };

private:
    Header* header_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_RTP_PACKET_H_