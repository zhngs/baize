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
        LOG_DEBUG << "BufferPool size:" << buffers_.size();

        auto buf = buffers_.AllocBuffer();
        int rn = stream_->AsyncRecvFrom(
            buf->write_index(), buf->writable_bytes(), &peeraddr);

        if (rn < 0) {
            LOG_ERROR << "webrtc server accept failed";
            return PeerConnectionSptr();
        }
        LOG_DEBUG << "recvfrom " << peeraddr.ip_port() << " " << rn << " bytes";

        buf->AddReadableLength(rn);

        string remote = peeraddr.ip_port();
        if (connections_.find(remote) != connections_.end()) {
            LOG_DEBUG << remote << " is an old peerconnection";
            auto pc_wptr = connections_[remote];
            auto pc_sptr = pc_wptr.lock();
            if (pc_sptr) {
                pc_sptr->packets_.emplace_back(std::move(buf));
                pc_sptr->async_park_.ScheduleRead();
            }
            continue;
        } else {
            LOG_INFO << remote << " start new peerconnection";
            PeerConnectionSptr pc(
                new PeerConnection(stream_, peeraddr, this),
                [this](PeerConnection* p) { PeerConnectionDelete(p); });
            connections_[remote] = pc;
            return pc;
        }
    }
}

void WebRTCServer::PeerConnectionDelete(PeerConnection* pc)
{
    string remote = pc->addr_.ip_port();
    LOG_TRACE << remote << " peerconnection delete";
    connections_.erase(remote);
    delete pc;
}

}  // namespace net

}  // namespace baize
