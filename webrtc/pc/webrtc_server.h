#ifndef BAIZE_WEBRTC_SERVER_H_
#define BAIZE_WEBRTC_SERVER_H_

#include <map>

#include "net/mtu_buffer_pool.h"
#include "webrtc/pc/peer_connection.h"
#include "webrtc/pc/webrtc_settings.h"

namespace baize
{

namespace net
{

class WebRTCServer  // noncopyable
{
public:  // type
    using RoomMap = std::map<string, PeerConnectionWptr>;

public:  // special function
    WebRTCServer(uint16_t port);
    ~WebRTCServer();
    WebRTCServer(const WebRTCServer&) = delete;
    WebRTCServer& operator=(const WebRTCServer&) = delete;

public:  // normal function
    PeerConnectionSptr Accept();

    // getter
    RoomMap& room() { return connections_; }

private:  // private normal function
    void PeerConnectionDelete(PeerConnection* pc);

private:
    UdpStreamSptr stream_;
    RoomMap connections_;
    MTUBufferPool buffers_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_WEBRTC_SERVER_H_