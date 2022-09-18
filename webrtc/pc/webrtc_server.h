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
public:  // special function
    WebRTCServer(uint16_t port);
    ~WebRTCServer();
    WebRTCServer(const WebRTCServer&) = delete;
    WebRTCServer& operator=(const WebRTCServer&) = delete;

public:  // normal function
    PeerConnectionSptr Accept();

private:
    UdpStreamSptr stream_;
    std::map<string, PeerConnectionWptr> connections_;
    MTUBufferPool buffers_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_WEBRTC_SERVER_H_