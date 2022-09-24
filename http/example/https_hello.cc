#include "http/http_stream.h"
#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::net;

void HttpsConnection(HttpStreamSptr stream, SslConfig& config)
{
    int err = stream->UpgradeHttps(config);
    if (err < 0) return;

    while (1) {
        HttpRequest req;
        int rn = stream->AsyncRead(req);
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

        int wn = stream->AsyncWrite(rsp);
        if (wn != rsp.slice().size()) {
            LOG_ERROR << "http write failed";
            break;
        }
    }
}

void HttpsServer()
{
    TcpListener listener(6060);
    SslConfig config;
    int err = config.set_tls_server("./cert.crt", "./cert.key");
    if (err < 0) {
        LOG_ERROR << "set cert key failed";
        return;
    }

    while (1) {
        TcpStreamSptr stream = listener.AsyncAccept();
        runtime::current_loop()->Do([stream, &config] {
            HttpsConnection(HttpStream::New(stream), config);
        });
    }
}

int main(int argc, char* argv[])
{
    runtime::EventLoop loop;

    loop.Do(HttpsServer);

    loop.Start();
}