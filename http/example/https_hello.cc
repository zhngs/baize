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
        HttpMessage req;
        int rn = stream->AsyncRead(req);
        if (rn <= 0) {
            LOG_ERROR << "http read failed";
            break;
        }

        auto req_line = req.request_line();

        LOG_INFO << req.first_line_;
        for (auto& head : req.headers_) {
            LOG_INFO << "header: {" << head.first << ":" << head.second << "}";
        }
        LOG_INFO << "body: " << req.body_;

        HttpMessage::ResponseLine rsp_line = {
            HttpMessage::Version::kHttp11,
            HttpMessage::StatusCode::k200,
            HttpMessage::StatusDescription::kOK};
        string rsp_line_string = HttpMessage::MakeResponseLine(rsp_line);

        HttpMessage rsp;
        rsp.set_response_line(rsp_line_string);
        rsp.set_headers("Content-Type", "text/plain; charset=utf-8");
        rsp.set_headers("Content-Length", "5");
        rsp.set_body("hello");

        int wn = stream->AsyncWrite(rsp);
        if (wn < 0) {
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