#ifndef BAIZE_WEBRTC_SERVER_H_
#define BAIZE_WEBRTC_SERVER_H_

#include <map>

#include "http/http_stream.h"
#include "net/net_buffer_pool.h"
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
    using PacketUptr = BufferPool::BufferUptr;

    struct Message {
        InetAddress addr;
        Buffer packet;
    };

public:  // special function
    WebRTCServer();
    ~WebRTCServer();
    WebRTCServer(const WebRTCServer&) = delete;
    WebRTCServer& operator=(const WebRTCServer&) = delete;

public:  // normal function
    void StartMediaServer(uint16_t port);
    void StartSignalServer(uint16_t port);

    void AsyncSend(InetAddress addr, StringPiece packet);

    // getter
    RoomMap& room() { return connections_; }

private:  // private normal function
    void DispatchDataLoop();
    void HandleMedia(PeerConnectionSptr pc);
    void HandleSignal(TcpStreamSptr stream, SslConfig& config);
    void PeerConnectionDelete(PeerConnection* pc);

private:
    UdpStreamSptr stream_;
    RoomMap connections_;
    BufferPool buffers_;
    std::vector<Message> send_messages_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_WEBRTC_SERVER_H_