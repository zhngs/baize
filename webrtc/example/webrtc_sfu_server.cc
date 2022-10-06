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

        int rn = http.AsyncRead(req);
        if (rn <= 0) {
            LOG_ERROR << "http read failed";
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
        } else if (req_line.url.Find("sdp") != req_line.url.end()) {
            LOG_INFO << "body : " << req.body_;
            current_remote_sdp().set_remote_sdp(req.body_);

            rsp.set_response_line(sucess_rsp_line_string);
            rsp.set_headers("Content-Length", local_sdp_len);
            rsp.set_body(local_sdp);
            LOG_INFO << "body: " << rsp.body_;

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
            [stream, &config] { HttpConnection(stream, config); });
    }
}

void HandlePeerConnection(PeerConnectionSptr pc)
{
    LOG_INFO << "peerconnection start";
    while (1) {
        auto packet = pc->AsyncRead();
        LOG_DEBUG << "peerconnection read " << packet->length() << " bytes";
        int err =
            pc->ProcessPacket(StringPiece(packet->data(), packet->length()));
        if (err < 0) {
            LOG_ERROR << "peerconnection error";
            break;
        }
    }
}

void MediaServer()
{
    WebRTCServer server(6061);
    while (1) {
        PeerConnectionSptr pc = server.Accept();
        runtime::current_loop()->Do([pc] { HandlePeerConnection(pc); });
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