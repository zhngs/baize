#include "http/http_server.h"

#include "http/http_parser.h"
#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

HttpServer::HttpServer(uint16_t port, HttpHandler handler)
  : listener_(port), handler_(std::move(handler))
{
    LOG_INFO << "HttpServer listen in " << port;
}

HttpServer::~HttpServer() {}

void HttpServer::Start()
{
    runtime::current_loop()->Do([this] {
        listener_.Start();
        while (1) {
            net::TcpStreamSptr stream = listener_.AsyncAccept();
            LOG_INFO << "connection " << stream->peer_ip_port() << " accept";
            runtime::current_loop()->Do([this, stream] { TcpLayer(stream); });
        }
    });
}

void HttpServer::TcpLayer(TcpStreamSptr stream)
{
    Buffer read_buf, write_buf;
    while (1) {
        bool timeout = false;
        int rn = stream->AsyncRead(read_buf, 1000 * 3, timeout);
        if (timeout) {
            LOG_INFO << "connection " << stream->peer_ip_port() << " timeout";
            break;
        }
        if (rn <= 0) {
            LOG_INFO << "connection " << stream->peer_ip_port() << " read "
                     << rn;
            break;
        }

        LOG_WARN << "tcp read " << rn;

        // 进入Http层
        int err = HttpLayer(read_buf, write_buf);
        if (err < 0) {
            break;
        }

        if (write_buf.readable_bytes() > 0) {
            stream->AsyncWrite(write_buf.read_index(),
                               write_buf.readable_bytes());
            write_buf.TakeAll();
        }
    }
    LOG_INFO << "connection " << stream->peer_ip_port() << " close";
}

int HttpServer::HttpLayer(Buffer& read_buf, Buffer& write_buf)
{
    while (1) {
        HttpRequest req;
        int parsed_len = HttpRequestParse(read_buf.slice(), req);
        if (parsed_len < 0) {
            LOG_ERROR << "http request parse failed";
            return parsed_len;
        } else if (parsed_len == 0) {
            return parsed_len;
        } else {
            read_buf.Take(parsed_len);
        }

        HttpResponse rsp;
        handler_(req, rsp);

        write_buf.Append(rsp.slice());
    }
}

}  // namespace net

}  // namespace baize
