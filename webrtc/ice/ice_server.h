#ifndef BAIZE_ICE_SERVER_H_
#define BAIZE_ICE_SERVER_H_

#include <memory>

#include "webrtc/ice/stun_packet.h"

namespace baize
{

namespace net
{

class IceServer;
using IceServerUptr = std::unique_ptr<IceServer>;

class PeerConnection;

class IceServer  // noncopyable
{
public:
    using Uptr = std::unique_ptr<IceServer>;
    enum class IceState { NEW = 1, CONNECTED };

    static Uptr New(PeerConnection* pc, string password);

    IceServer(PeerConnection* pc, string password)
      : pc_(pc), ice_password_(password)
    {
    }
    ~IceServer();
    IceServer(const IceServer&) = delete;
    IceServer& operator=(const IceServer&) = delete;

    void ProcessStunPacket(StringPiece stun_packet);

    // getter
    bool is_connected() { return state_ == IceState::CONNECTED; }

private:
    PeerConnection* pc_;
    IceState state_ = IceState::NEW;
    string ice_password_;
    Buffer send_buf_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_ICE_SERVER_H_