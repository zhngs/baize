#include "http/http_server.h"

#include "http/http_parser.h"
#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

HttpListener::HttpListener(uint16_t port) : listener_(port)
{
    LOG_INFO << "HttpServer listen in " << port;
}

HttpListener::~HttpListener() {}

HttpStreamSptr HttpListener::AsyncAccept()
{
    net::TcpStreamSptr stream = listener_.AsyncAccept();
    LOG_INFO << "connection " << stream->peer_ip_port() << " accept";
    return HttpStreamSptr(std::make_shared<HttpStream>(stream));
}

HttpStream::HttpStream(TcpStreamSptr stream) : stream_(stream) {}

HttpStream::~HttpStream() {}

int HttpStream::AsyncRead(HttpRequest& req)
{
    while (1) {
        int parsed_len = HttpRequestParse(read_buf_.slice(), req);
        if (parsed_len < 0) {
            LOG_ERROR << "http request parse failed";
            return parsed_len;
        } else if (parsed_len == 0) {
            bool timeout = false;
            int rn = stream_->AsyncRead(read_buf_, 1000 * 3, timeout);
            if (timeout) {
                LOG_INFO << "connection " << stream_->peer_ip_port()
                         << " timeout";
                return -1;
            }
            if (rn <= 0) {
                LOG_INFO << "connection " << stream_->peer_ip_port() << " read "
                         << rn;
                return rn;
            }
        } else {
            read_buf_.Take(parsed_len);
            return parsed_len;
        }
    }
}

int HttpStream::AsyncWrite(HttpResponseBuilder& rsp)
{
    StringPiece content = rsp.slice();
    if (content.size() <= 0) {
        return -1;
    }

    return stream_->AsyncWrite(content.data(), content.size());
}

}  // namespace net

}  // namespace baize
