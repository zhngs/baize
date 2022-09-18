#ifndef BAIZE_ICE_SERVER_H_
#define BAIZE_ICE_SERVER_H_

#include "net/udp_stream.h"
#include "webrtc/ice/stun_packet.h"

namespace baize
{

namespace net
{

class IceServer;
using IceServerUptr = std::unique_ptr<IceServer>;

class IceServer  // noncopyable
{
public:
    using Uptr = std::unique_ptr<IceServer>;
    enum class IceState { NEW = 1, CONNECTED };

    static Uptr New(string password, UdpStreamSptr stream, InetAddress addr);

    IceServer(string password, UdpStreamSptr stream, InetAddress addr)
      : ice_password_(password), stream_(stream), dest_addr_(addr)
    {
    }
    ~IceServer();
    IceServer(const IceServer&) = delete;
    IceServer& operator=(const IceServer&) = delete;

    void ProcessStunPacket(StringPiece stun_packet);

    // getter
    bool is_connected() { return state_ == IceState::CONNECTED; }

private:
    IceState state_ = IceState::NEW;
    string ice_password_;

    UdpStreamSptr stream_;
    InetAddress dest_addr_;
    Buffer send_buf_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_ICE_SERVER_H_