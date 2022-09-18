#include "webrtc/pc/webrtc_server.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

WebRTCServer::WebRTCServer(uint16_t port) : stream_(UdpStream::AsServer(port))
{
    WebRTCSettings::Initialize();
}

WebRTCServer::~WebRTCServer() {}

PeerConnectionSptr WebRTCServer::Accept()
{
    InetAddress peeraddr;

    while (1) {
        LOG_INFO << "MTUBufferPool size:" << buffers_.size();

        auto buf = buffers_.PacketBuffer();
        int rn =
            stream_->AsyncRecvFrom(buf->data(), buf->capacity(), &peeraddr);

        if (rn < 0) {
            LOG_ERROR << "webrtc server accept failed";
            return PeerConnectionSptr();
        }
        LOG_INFO << "recvfrom " << peeraddr.ip_port() << " " << rn << " bytes";

        buf->add(rn);

        string remote = peeraddr.ip_port();
        if (connections_.find(remote) != connections_.end()) {
            auto pc_wptr = connections_[remote];
            auto pc_sptr = pc_wptr.lock();
            if (pc_sptr) {
                pc_sptr->packets_.emplace_back(std::move(buf));
                pc_sptr->async_park_.ScheduleRead();
            }
            continue;
        } else {
            PeerConnectionSptr pc = PeerConnection::New(stream_, peeraddr);
            connections_[remote] = pc;
            return pc;
        }
    }
}

}  // namespace net

}  // namespace baize
