#include "http/https_server.h"

#include "http/http_parser.h"
#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

HttpsListener::HttpsListener(uint16_t port) : listener_(port)
{
    LOG_INFO << "HttpServer listen in " << port;
    listener_.Start();
}

HttpsListener::~HttpsListener() {}

int HttpsListener::set_cert_key(string cert, string key)
{
    return config_.set_tls_server(cert, key);
}

HttpsStreamSptr HttpsListener::AsyncAccept()
{
    net::TcpStreamSptr stream = listener_.AsyncAccept();
    LOG_INFO << "connection " << stream->peer_ip_port() << " accept";
    return HttpsStreamSptr(std::make_shared<HttpsStream>(config_, stream));
}

HttpsStream::HttpsStream(SslConfig& config, TcpStreamSptr stream)
  : tcp_stream_(stream), config_(config)
{
}

HttpsStream::~HttpsStream() {}

int HttpsStream::TlsHandshake()
{
    tls_stream_ = TlsStream::AsServer(config_, tcp_stream_);
    if (tls_stream_) {
        return 0;
    } else {
        return -1;
    }
}

int HttpsStream::AsyncRead(HttpRequest& req)
{
    while (1) {
        int parsed_len = HttpRequestParse(read_buf_.slice(), req);
        if (parsed_len < 0) {
            LOG_ERROR << "http request parse failed";
            return parsed_len;
        } else if (parsed_len == 0) {
            // fixme: 这里多了一次拷贝，并且不支持超时
            char buf[1024];
            int rn = tls_stream_->AsyncRead(buf, sizeof(buf));
            if (rn <= 0) {
                LOG_INFO << "connection " << tcp_stream_->peer_ip_port()
                         << " read " << rn;
                return rn;
            }
            read_buf_.Append(buf, rn);
        } else {
            read_buf_.Take(parsed_len);
            return parsed_len;
        }
    }
}

int HttpsStream::AsyncWrite(HttpResponseBuilder& rsp)
{
    StringPiece content = rsp.slice();
    if (content.size() <= 0) {
        return -1;
    }

    return tls_stream_->AsyncWrite(content.data(), content.size());
}

}  // namespace net

}  // namespace baize
