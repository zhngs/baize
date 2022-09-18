#include "http/https_server.h"
#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::net;

void HttpsConnection(HttpsStreamSptr https)
{
    int err = https->TlsHandshake();
    if (err < 0) return;

    while (1) {
        HttpRequest req;
        int rn = https->AsyncRead(req);
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

        int wn = https->AsyncWrite(rsp);
        if (wn != rsp.slice().size()) {
            LOG_ERROR << "http write failed";
            break;
        }
    }
}

void HttpsServer()
{
    HttpsListener listener(6060);
    int err = listener.set_cert_key("./cert.crt", "./cert.key");
    if (err < 0) {
        LOG_ERROR << "set cert key failed";
        return;
    }

    while (1) {
        HttpsStreamSptr stream = listener.AsyncAccept();
        runtime::current_loop()->Do([stream] { HttpsConnection(stream); });
    }
}

int main(int argc, char* argv[])
{
    runtime::EventLoop loop;

    loop.Do(HttpsServer);

    loop.Start();
}