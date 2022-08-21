#ifndef BAIZE_SDP_MESSAGE_H_
#define BAIZE_SDP_MESSAGE_H_

#include <map>

#include "util/string_piece.h"

namespace baize
{

namespace net
{

struct TrackSdp  // copyable
{
    string media_type_;
    string net_protocol_;
    std::vector<int> payload_type_;
    std::map<int, string> extmap_;
    std::map<int, string> rtpmap_;
    std::map<int, string> rtcp_fb_;
    std::map<int, string> fmtp_;
    std::vector<string> ssrc_group_;
};

struct NetSdp  // copyable
{
    string ice_ufrag_;
    string ice_pwd_;
    string finger_print_;
    string ice_option_;
    string setup_;
};

class SdpMessage  // copyable
{
public:
public:
    string cname_;
    std::vector<TrackSdp> tracks_;
    NetSdp net_;
};

int SdpParse(StringPiece message, SdpMessage& sdp);

}  // namespace net

}  // namespace baize

#endif  // BAIZE_SDP_MESSAGE_H_