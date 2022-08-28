#include "http/file_reader.h"
#include "http/http_server.h"
#include "log/logger.h"
#include "runtime/event_loop.h"
#include "webrtc/sdp/sdp_message.h"

using namespace baize;
using namespace baize::net;

void HttpEntry(const HttpRequest& req, HttpResponseBuilder& rsp)
{
    if (req.path_ == "/") {
        FileReader reader("p2p.html");
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

        SdpMessage sdp;
        SdpParse(req.body_, sdp);

        rsp.AppendResponseLine("HTTP/1.1", "200", "OK");
        rsp.AppendBody(req.body_);
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

void HttpServer()
{
    HttpListener listener(6060);

    while (1) {
        HttpStreamSptr stream = listener.AsyncAccept();
        runtime::current_loop()->Do([stream] { HttpConnection(stream); });
    }
}

int main(int argc, char* argv[])
{
    log::Logger::set_loglevel(log::Logger::INFO);
    runtime::EventLoop loop;

    loop.Do(HttpServer);

    loop.Start();
}