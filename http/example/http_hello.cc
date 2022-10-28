#include "http/http_stream.h"
#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::net;

void HttpConnection(HttpStreamSptr stream)
{
    while (1) {
        bool timeout = false;
        HttpMessage req;
        int rn = stream->AsyncRead(req, 5000, timeout);
        if (timeout) {
            LOG_ERROR << "http read timeout";
            break;
        }
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

void HttpServer()
{
    TcpListener listener(6060);

    while (1) {
        TcpStreamSptr stream = listener.AsyncAccept();
        runtime::current_loop()->Do(
            [stream] { HttpConnection(HttpStream::New(stream)); });
    }
}

int main(int argc, char* argv[])
{
    runtime::EventLoop loop;

    loop.Do(HttpServer);

    loop.Start();
}