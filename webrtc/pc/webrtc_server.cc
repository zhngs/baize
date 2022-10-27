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
        bool timeout = false;
        int rn = stream_->AsyncRecvFrom(
            buf->write_index(), buf->writable_bytes(), &peeraddr, 1, timeout);

        if (!timeout) {
            if (rn < 0) {
                LOG_ERROR << "webrtc server accept failed";
                return PeerConnectionSptr();
            }
            LOG_DEBUG << "recvfrom " << peeraddr.ip_port() << " " << rn
                      << " bytes";

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
                    new PeerConnection(this, peeraddr),
                    [this](PeerConnection* p) { PeerConnectionDelete(p); });
                connections_[remote] = pc;
                return pc;
            }
        }

        if (!send_messages_.empty()) {
            for (auto& message : send_messages_) {
                int wn = stream_->AsyncSendto(message.packet.read_index(),
                                              message.packet.readable_bytes(),
                                              message.addr);
                if (wn != message.packet.readable_bytes()) {
                    LOG_ERROR << "webrtc server write error";
                }
            }
            send_messages_.clear();
        }
    }
}

void WebRTCServer::AsyncSend(InetAddress addr, StringPiece packet)
{
    int wn = stream_->socket()->SendTo(packet.data(), packet.size(), addr);
    if (wn != packet.size()) {
        Message message;
        message.addr = addr;
        message.packet.Append(packet);
        send_messages_.push_back(std::move(message));
    }
}

void WebRTCServer::PeerConnectionDelete(PeerConnection* pc)
{
    string remote = pc->addr_.ip_port();
    LOG_INFO << remote << " peerconnection delete";
    connections_.erase(remote);
    delete pc;
}

}  // namespace net

}  // namespace baize
