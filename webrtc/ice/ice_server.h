#ifndef BAIZE_ICE_SERVER_H_
#define BAIZE_ICE_SERVER_H_

#include "net/udp_stream.h"
#include "webrtc/ice/stun_packet.h"

namespace baize
{

namespace net
{

void ProcessStunPacket(StunPacket& packet,
                       UdpStreamSptr stream,
                       InetAddress& addr,
                       string password);

}  // namespace net

}  // namespace baize

#endif  // BAIZE_ICE_SERVER_H_