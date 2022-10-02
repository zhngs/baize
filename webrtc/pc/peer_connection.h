#ifndef BAIZE_PEER_CONNECTION_H_
#define BAIZE_PEER_CONNECTION_H_

#include "net/mtu_buffer_pool.h"
#include "net/udp_stream.h"
#include "runtime/event_loop.h"
#include "webrtc/dtls/dtls_transport.h"
#include "webrtc/ice/ice_server.h"
#include "webrtc/rtp/rtcp_packet.h"
#include "webrtc/rtp/srtp_session.h"
#include "webrtc/sdp/sdp_message.h"

namespace baize
{

namespace net
{

class PeerConnection;
using PeerConnectionSptr = std::shared_ptr<PeerConnection>;
using PeerConnectionWptr = std::weak_ptr<PeerConnection>;

class PeerConnection  // noncopyable
{
public:  // types and constant
    friend class WebRTCServer;
    using Packet = MTUBufferPool::PacketUptr;

public:  // special function
    PeerConnection(UdpStreamSptr stream, InetAddress addr);
    ~PeerConnection();
    PeerConnection(const PeerConnection&) = delete;
    PeerConnection& operator=(const PeerConnection&) = delete;

public:  // normal function
    Packet AsyncRead();
    int ProcessPacket(StringPiece packet);

private:
    UdpStreamSptr stream_;
    InetAddress addr_;
    std::vector<Packet> packets_;

    IceServerUptr ice_;
    DtlsTransportUptr dtls_;
    SrtpSessionUptr srtp_send_session_;
    SrtpSessionUptr srtp_recv_session_;

    runtime::AsyncPark async_park_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_PEER_CONNECTION_H_