#include "http/file_reader.h"
#include "http/http_server.h"
#include "log/logger.h"
#include "net/udp_stream.h"
#include "runtime/event_loop.h"
#include "webrtc/ice/ice_server.h"
#include "webrtc/ice/stun_packet.h"
#include "webrtc/sdp/sdp_message.h"

using namespace baize;
using namespace baize::net;

string g_password;

void HttpEntry(const HttpRequest& req, HttpResponseBuilder& rsp)
{
    if (req.path_ == "/") {
        FileReader reader("demo.html");
        StringPiece file = reader.ReadAll();
        string len = std::to_string(file.size());

        rsp.AppendResponseLine("HTTP/1.1", "200", "OK");
        rsp.AppendHeader("Content-Length", len);
        rsp.AppendBody(file);
    } else if (req.path_.Find("sdp") != req.path_.end()) {
        // LOG_INFO << req.all_data_;
        // LOG_INFO << "method: " << req.method_;
        // LOG_INFO << "path: " << req.path_;
        // LOG_INFO << "version: " << req.version_;
        // for (auto& head : req.headers_) {
        //     LOG_INFO << "header: {" << head.first << ":" << head.second <<
        //     "}";
        // }
        // LOG_INFO << "body len : " << req.body_.size();
        LOG_INFO << "body : " << req.body_;

        SdpMessage remote_sdp;
        remote_sdp.set_remote_sdp(req.body_);

        g_password = remote_sdp.net_.ice_pwd_;

        SdpMessage local_sdp;
        local_sdp.net_.ip_ = "101.43.183.201";
        local_sdp.net_.port_ = 6061;
        local_sdp.net_.ice_ufrag_ = "2eaP";
        local_sdp.net_.ice_pwd_ = "0HK9scLUJ8kv2TiuDAPHccjb";
        local_sdp.net_.ice_option_ = "ice-lite";
        local_sdp.net_.finger_print_ =
            "D0:7C:21:96:77:95:8A:7B:BD:13:B3:84:FB:CB:80:03:0C:F5:5B:AD:DD:04:"
            "1A:07:0E:44:C0:26:80:BB:D6:6A";

        rsp.AppendResponseLine("HTTP/1.1", "200", "OK");
        rsp.AppendBody(local_sdp.local_sdp());
    } else {
        rsp.AppendResponseLine("HTTP/1.1", "200", "OK");
        rsp.AppendEmptyBody();
    }
}

void HttpConnection(HttpStreamSptr http)
{
    while (1) {
        HttpRequest req;
        int rn = http->AsyncRead(req);
        if (rn <= 0) {
            LOG_ERROR << "http read failed";
            break;
        }

        HttpResponseBuilder rsp;
        HttpEntry(req, rsp);

        int wn = http->AsyncWrite(rsp);
        if (wn != rsp.slice().size()) {
            LOG_ERROR << "http write failed";
            break;
        }
    }
}

void SignalServer()
{
    HttpListener listener(6060);

    while (1) {
        HttpStreamSptr stream = listener.AsyncAccept();
        runtime::current_loop()->Do([stream] { HttpConnection(stream); });
    }
}

void MediaServer()
{
    char buf[1500];
    InetAddress addr;
    UdpStreamSptr stream = UdpStream::AsServer(6061);
    while (1) {
        int rn = stream->AsyncRecvFrom(buf, sizeof(buf), &addr);
        if (rn < 0) {
            LOG_ERROR << "mediaserver read failed";
            break;
        }
        LOG_INFO << "recvfrom " << addr.ip_port() << " " << rn << " bytes";
        LOG_INFO << log::DumpHexFormat(StringPiece(buf, rn));
        if (StunPacket::IsStun(StringPiece(buf, rn))) {
            StunPacket stun_packet;
            int err = stun_packet.Parse(StringPiece(buf, rn));
            if (!err) {
                stun_packet.dump();
                ProcessStunPacket(stun_packet, stream, addr, g_password);
            } else {
                LOG_ERROR << "stun packet parse failed";
            }
        }
    }
}

int main(int argc, char* argv[])
{
    log::Logger::set_loglevel(log::Logger::INFO);
    runtime::EventLoop loop;

    loop.Do(SignalServer);
    loop.Do(MediaServer);

    loop.Start();
}