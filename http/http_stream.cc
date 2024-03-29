#include "http/http_stream.h"

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

int HttpStream::AsyncRead(HttpMessage& message, int ms, bool& timeout)
{
    while (1) {
        int parsed_len = message.Decode(read_buf_);
        if (parsed_len < 0) {
            LOG_ERROR << "http request parse failed";
            return parsed_len;
        } else if (parsed_len == 0) {
            if (tls_stream_) {  // https
                int rn = tls_stream_->AsyncRead(read_buf_.write_index(),
                                                read_buf_.writable_bytes(),
                                                ms,
                                                timeout);
                if (timeout) {
                    LOG_INFO << "connection " << tcp_stream_->peer_ip_port()
                             << " timeout";
                    return -1;
                }

                if (rn <= 0) {
                    LOG_INFO << "connection " << tcp_stream_->peer_ip_port()
                             << " read " << rn;
                    return rn;
                } else {
                    read_buf_.AddReadableLength(rn);
                }
            } else {  // http
                int rn = tcp_stream_->AsyncRead(read_buf_, ms, timeout);
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
            return parsed_len;
        }
    }
}

int HttpStream::AsyncWrite(HttpMessage& message)
{
    message.Encode(write_buf_);
    if (tls_stream_) {
        int len = tls_stream_->AsyncWrite(write_buf_.read_index(),
                                          write_buf_.readable_bytes());
        if (len != write_buf_.readable_bytes()) return -1;
    } else {
        int len = tcp_stream_->AsyncWrite(write_buf_.read_index(),
                                          write_buf_.readable_bytes());
        if (len != write_buf_.readable_bytes()) return -1;
    }
    write_buf_.TakeAll();
    return 0;
}

}  // namespace net

}  // namespace baize
