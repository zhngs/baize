#ifndef BAIZE_PEER_CONNECTION_H_
#define BAIZE_PEER_CONNECTION_H_

#include "net/net_buffer_pool.h"
#include "net/udp_stream.h"
#include "runtime/event_loop.h"
#include "webrtc/dtls/dtls_transport.h"
#include "webrtc/ice/ice_server.h"
#include "webrtc/rtp/rtcp_packet.h"
#include "webrtc/rtp/rtp_packet.h"
#include "webrtc/rtp/srtp_session.h"
#include "webrtc/sdp/sdp_message.h"

namespace baize
{

namespace net
{

class PeerConnection;
using PeerConnectionSptr = std::shared_ptr<PeerConnection>;
using PeerConnectionWptr = std::weak_ptr<PeerConnection>;

class WebRTCServer;

class PeerConnection  // noncopyable
{
public:  // types and constant
    friend class WebRTCServer;

    using PacketUptr = BufferPool::BufferUptr;
    using PacketUptrVector = std::vector<PacketUptr>;

public:  // special function
    PeerConnection(WebRTCServer* webrtc_server);
    ~PeerConnection();
    PeerConnection(const PeerConnection&) = delete;
    PeerConnection& operator=(const PeerConnection&) = delete;

public:  // normal function
    PacketUptrVector AsyncRead(int ms, bool& timeout);
    void AsyncSend(StringPiece packet);

    int ProcessPacket(StringPiece packet);

    void set_sdp(const SdpMessage& sdp) { sdp_ = sdp; }

    void set_addr(const InetAddress& addr) { addr_ = addr; }
    const InetAddress& addr() { return addr_; }

private:  // private normal function
    int ProcessRtcp(StringPiece packet);

    void SendPli();
    int OnPliTimer();

private:
    WebRTCServer* webrtc_server_ = nullptr;
    InetAddress addr_;
    std::vector<PacketUptr> packets_;

    time::Timer pli_timer_;

    bool established_ = false;
    SdpMessage sdp_;
    IceServerUptr ice_;
    DtlsTransportUptr dtls_;
    SrtpSessionUptr srtp_send_session_;
    SrtpSessionUptr srtp_recv_session_;

    runtime::AsyncPark async_park_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_PEER_CONNECTION_H_