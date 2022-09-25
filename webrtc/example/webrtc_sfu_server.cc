#include "http/file_reader.h"
#include "http/http_stream.h"
#include "log/logger.h"
#include "net/udp_stream.h"
#include "runtime/event_loop.h"
#include "webrtc/pc/webrtc_server.h"

using namespace baize;
using namespace baize::net;

void HttpEntry(HttpMessage& req, HttpMessage& rsp)
{
    auto req_line = req.request_line();

    HttpMessage::ResponseLine rsp_line = {HttpMessage::Version::kHttp11,
                                          HttpMessage::StatusCode::k200,
                                          HttpMessage::StatusDescription::kOK};
    string rsp_line_string = HttpMessage::MakeResponseLine(rsp_line);

    if (req_line.url == "/") {
        FileReader reader("demo.html");
        StringPiece file = reader.ReadAll();
        string len = std::to_string(file.size());

        rsp.set_response_line(rsp_line_string);
        rsp.set_headers("Content-Length", len);
        rsp.set_body(file);
    } else if (req_line.url.Find("sdp") != req_line.url.end()) {
        LOG_INFO << "body : " << req.body_;
        SdpMessage remote_sdp;
        remote_sdp.set_remote_sdp(req.body_);

        rsp.set_response_line(rsp_line_string);
        rsp.set_body(WebRTCSettings::local_sdp());
    } else {
        rsp.set_response_line(rsp_line_string);
    }
}

void HttpConnection(TcpStreamSptr stream)
{
    HttpStream http(stream);
    while (1) {
        HttpMessage req;
        int rn = http.AsyncRead(req);
        if (rn <= 0) {
            LOG_ERROR << "http read failed";
            break;
        }

        HttpMessage rsp;
        HttpEntry(req, rsp);

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

    while (1) {
        auto stream = listener.AsyncAccept();
        runtime::current_loop()->Do([stream] { HttpConnection(stream); });
    }
}

void PeerConn(PeerConnectionSptr pc)
{
    LOG_INFO << "peerconnection start";
    while (1) {
        auto packet = pc->AsyncRead();
        LOG_INFO << "peerconnection read " << packet->length() << " bytes";
        pc->ProcessPacket(StringPiece(packet->data(), packet->length()));
    }
}

void MediaServer()
{
    // char buf[1500];
    // InetAddress addr;
    // UdpStreamSptr stream = UdpStream::AsServer(6061);

    // bool only_once = false;
    // PeerConnection::Sptr pc;

    // WebRTCSettings::Initialize();

    // while (1) {
    //     int rn = stream->AsyncRecvFrom(buf, sizeof(buf), &addr);
    //     if (rn < 0) {
    //         LOG_ERROR << "mediaserver read failed";
    //         break;
    //     }
    //     LOG_INFO << "recvfrom " << addr.ip_port() << " " << rn << " bytes";

    //     if (!only_once) {
    //         pc = PeerConnection::New(stream, addr);
    //         only_once = true;
    //     }

    //     int err = pc->ProcessPacket(StringPiece(buf, rn));
    //     if (err < 0) {
    //         LOG_ERROR << "peerconnection process failed";
    //         break;
    //     }
    // }

    WebRTCServer server(6061);
    while (1) {
        PeerConnectionSptr pc = server.Accept();
        runtime::current_loop()->Do([pc] { PeerConn(pc); });
    }
}

int main(int argc, char* argv[])
{
    set_log_trace();
    runtime::EventLoop loop;

    loop.Do(SignalServer);
    loop.Do(MediaServer);

    loop.Start();
}