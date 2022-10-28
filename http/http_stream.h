#ifndef BAIZE_HTTP_STREAM_H_
#define BAIZE_HTTP_STREAM_H_

#include "crypto/tls_stream.h"
#include "http/http_message.h"
#include "net/tcp_listener.h"

namespace baize
{

namespace net
{

class HttpStream;
using HttpStreamSptr = std::shared_ptr<HttpStream>;

class HttpStream  // noncopyable
{
public:
    static HttpStreamSptr New(TcpStreamSptr stream);

public:
    explicit HttpStream(TcpStreamSptr stream);
    ~HttpStream();
    HttpStream(const HttpStream&) = delete;
    HttpStream& operator=(const HttpStream&) = delete;

    int UpgradeHttps(SslConfig& config);

    int AsyncRead(HttpMessage& message, int ms, bool& timeout);
    int AsyncWrite(HttpMessage& message);

private:
    Buffer read_buf_;
    Buffer write_buf_;
    TcpStreamSptr tcp_stream_;
    TlsStreamSptr tls_stream_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_HTTP_STREAM_H_
