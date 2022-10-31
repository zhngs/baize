#include "http/file_reader.h"
#include "http/http_stream.h"
#include "log/logger.h"
#include "net/udp_stream.h"
#include "runtime/event_loop.h"
#include "webrtc/pc/webrtc_server.h"
#include "webrtc/rtp/rtcp_packet.h"

using namespace baize;
using namespace baize::net;

void HttpConnection(TcpStreamSptr stream, SslConfig& config)
{
    HttpStream http(stream);
    int err = http.UpgradeHttps(config);
    if (err < 0) return;

    FileReader reader("demo.html");
    StringPiece demo_file = reader.ReadAll();
    string demo_file_len = std::to_string(demo_file.size());

    string local_sdp = WebRTCSettings::local_sdp();
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
            current_pub_sdp().set_remote_sdp(req.body_);

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

void SignalServer()
{
    TcpListener listener(6060);
    SslConfig config;
    int err = config.set_tls_server("./cert.crt", "./cert.key");
    if (err < 0) {
        LOG_ERROR << "set cert key failed";
        return;
    }

    while (1) {
        auto stream = listener.AsyncAccept();
        runtime::current_loop()->Do(
            [stream, &config] { HttpConnection(stream, config); },
            std::string("http") + stream->peer_ip_port());
    }
}

void HandlePeerConnection(PeerConnectionSptr pc)
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

void MediaServer()
{
    WebRTCServer server(6061);
    while (1) {
        PeerConnectionSptr pc = server.Accept();
        if (pc) {
            runtime::current_loop()->Do(
                [pc] { HandlePeerConnection(pc); },
                std::string("pc") + pc->addr().ip_port());
        }
    }
}

int main(int argc, char* argv[])
{
    set_log_info();
    runtime::EventLoop loop;

    loop.Do(SignalServer);
    loop.Do(MediaServer);

    loop.Start();
}