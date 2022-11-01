#include "webrtc/pc/webrtc_server.h"

#include "http/file_reader.h"
#include "log/logger.h"

namespace baize
{

namespace net
{

WebRTCServer::WebRTCServer() { WebRTCSettings::Initialize(); }

WebRTCServer::~WebRTCServer() {}

void WebRTCServer::StartMediaServer(uint16_t port)
{
    stream_ = UdpStream::AsServer(port);
    if (!stream_) {
        LOG_ERROR << "start media server failed";
        return;
    }

    while (1) {
        PeerConnectionSptr pc = Accept();
        if (pc) {
            runtime::current_loop()->Do(
                [this, pc] { HandleMedia(pc); },
                std::string("pc") + pc->addr().ip_port());
        }
    }
}

void WebRTCServer::HandleMedia(PeerConnectionSptr pc)
{
    LOG_INFO << "peerconnection start";
    while (1) {
        bool timeout = false;
        auto packets = pc->AsyncRead(5000, timeout);

        if (timeout) {
            LOG_INFO << "pc timeout";
            break;
        }

        for (auto& packet : packets) {
            LOG_DEBUG << "peerconnection read " << packet->readable_bytes()
                      << " bytes";
            int err = pc->ProcessPacket(packet->slice());

            if (err < 0) {
                LOG_ERROR << "peerconnection error";
                return;
            }
        }
    }
}

void WebRTCServer::StartSignalServer(uint16_t port)
{
    TcpListener listener(port);
    SslConfig config;
    int err = config.set_tls_server("./cert.crt", "./cert.key");
    if (err < 0) {
        LOG_ERROR << "set cert key failed";
        return;
    }

    while (1) {
        auto stream = listener.AsyncAccept();
        runtime::current_loop()->Do(
            [this, stream, &config] { HandleSignal(stream, config); },
            std::string("http") + stream->peer_ip_port());
    }
}

void WebRTCServer::HandleSignal(TcpStreamSptr stream, SslConfig& config)
{
    HttpStream http(stream);
    int err = http.UpgradeHttps(config);
    if (err < 0) return;

    FileReader reader("demo.html");
    StringPiece demo_file = reader.ReadAll();
    string demo_file_len = std::to_string(demo_file.size());

    string local_sdp = WebRTCSettings::local_sdp_string();
    string local_sdp_len = std::to_string(local_sdp.size());

    HttpMessage::ResponseLine success_rsp_line = {
        HttpMessage::Version::kHttp11,
        HttpMessage::StatusCode::k200,
        HttpMessage::StatusDescription::kOK};
    string sucess_rsp_line_string =
        HttpMessage::MakeResponseLine(success_rsp_line);

    while (1) {
        HttpMessage req;
        HttpMessage rsp;

        bool timeout = false;
        int rn = http.AsyncRead(req, 5000, timeout);
        if (timeout) {
            LOG_ERROR << "http read timeout";
            break;
        }
        if (rn == 0) {
            LOG_ERROR << "http connection close";
            break;
        } else if (rn < 0) {
            LOG_ERROR << "http connection error";
            break;
        }

        LOG_INFO << "req first line: " << req.first_line_;
        auto req_line = req.request_line();
        LOG_INFO << "req url: " << req_line.url;
        if (req_line.url == "/") {
            rsp.set_response_line(sucess_rsp_line_string);
            rsp.set_headers("Content-Type", "text/html; charset=utf-8");
            rsp.set_headers("Content-Length", demo_file_len);
            rsp.set_body(demo_file);
        } else if (req_line.url.find("pub") != req_line.url.npos) {
            LOG_INFO << "pub body : " << req.body_;

            SdpMessage remote_sdp;
            remote_sdp.Decode(req.body_);

            string username = remote_sdp.net_.ice_ufrag_;
            assert(!username.empty());
            assert(connections_.find(username) == connections_.end());
            connections_[username] = std::make_shared<PeerConnection>(this);

            rsp.set_response_line(sucess_rsp_line_string);
            rsp.set_headers("Content-Length", local_sdp_len);
            rsp.set_body(local_sdp);
            LOG_INFO << "body: " << rsp.body_;

        } else if (req_line.url.find("sub") != req_line.url.npos) {
            // do nothing
        } else {
            rsp.set_response_line(sucess_rsp_line_string);
        }

        int wn = http.AsyncWrite(rsp);
        if (wn < 0) {
            LOG_ERROR << "http write failed";
            break;
        }
    }
}

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
