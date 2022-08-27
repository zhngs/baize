#include "http/http_server.h"
#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::net;

void HttpConnection(HttpStreamSptr http)
{
    while (1) {
        HttpRequest req;
        int rn = http->AsyncRead(req);
        if (rn <= 0) {
            LOG_ERROR << "http read failed";
            break;
        }

        LOG_INFO << req.all_data_;
        LOG_INFO << "method: " << req.method_;
        LOG_INFO << "path: " << req.path_;
        LOG_INFO << "version: " << req.version_;
        for (auto& head : req.headers_) {
            LOG_INFO << "header: {" << head.first << ":" << head.second << "}";
        }
        LOG_INFO << "body: " << req.body_;

        HttpResponseBuilder rsp;
        rsp.AppendResponseLine("HTTP/1.1", "200", "OK");
        rsp.AppendHeader("Content-Type", "text/plain; charset=utf-8");
        rsp.AppendHeader("Content-Length", "5");
        rsp.AppendBody("hello");

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