#ifndef BAIZE_PEER_CONNECTION_H_
#define BAIZE_PEER_CONNECTION_H_

#include "net/udp_stream.h"
#include "webrtc/dtls/dtls_transport.h"
#include "webrtc/ice/ice_server.h"
#include "webrtc/sdp/sdp_message.h"

namespace baize
{

namespace net
{

class PeerConnection  // noncopyable
{
public:
    using Sptr = std::shared_ptr<PeerConnection>;

    static Sptr New(UdpStreamSptr stream, InetAddress& addr);

    PeerConnection(UdpStreamSptr stream, InetAddress addr);
    ~PeerConnection();
    PeerConnection(const PeerConnection&) = delete;
    PeerConnection& operator=(const PeerConnection&) = delete;

    int ProcessPacket(StringPiece packet);

private:
    UdpStreamSptr stream_;
    InetAddress addr_;

    IceServer::Uptr ice_;
    DtlsTransport::Uptr dtls_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_PEER_CONNECTION_H_