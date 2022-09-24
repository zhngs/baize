#include "http/http_stream.h"

#include "http/http_parser.h"
#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

HttpStreamSptr HttpStream::New(TcpStreamSptr stream)
{
    return std::make_shared<HttpStream>(stream);
}

HttpStream::HttpStream(TcpStreamSptr stream) : tcp_stream_(stream) {}

HttpStream::~HttpStream() {}

int HttpStream::UpgradeHttps(SslConfig& config)
{
    tls_stream_ = TlsStream::AsServer(config, tcp_stream_);
    if (tls_stream_) {
        return 0;
    } else {
        return -1;
    }
}

int HttpStream::AsyncRead(HttpRequest& req)
{
    while (1) {
        int parsed_len = HttpRequestParse(read_buf_.slice(), req);
        if (parsed_len < 0) {
            LOG_ERROR << "http request parse failed";
            return parsed_len;
        } else if (parsed_len == 0) {
            if (tls_stream_) {  // https
                // fixme: 这里多了一次拷贝，并且不支持超时
                char buf[1024];
                int rn = tls_stream_->AsyncRead(buf, sizeof(buf));
                if (rn <= 0) {
                    LOG_INFO << "connection " << tcp_stream_->peer_ip_port()
                             << " read " << rn;
                    return rn;
                }
                read_buf_.Append(buf, rn);
            } else {  // http
                bool timeout = false;
                int rn = tcp_stream_->AsyncRead(read_buf_, 1000 * 3, timeout);
                if (timeout) {
                    LOG_INFO << "connection " << tcp_stream_->peer_ip_port()
                             << " timeout";
                    return -1;
                }
                if (rn <= 0) {
                    LOG_INFO << "connection " << tcp_stream_->peer_ip_port()
                             << " read " << rn;
                    return rn;
                }
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

    if (tls_stream_) {
        return tls_stream_->AsyncWrite(content.data(), content.size());
    } else {
        return tcp_stream_->AsyncWrite(content.data(), content.size());
    }
}

}  // namespace net

}  // namespace baize
