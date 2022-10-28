#ifndef BAIZE_TLS_STREAM_H_
#define BAIZE_TLS_STREAM_H_

#include "crypto/ssl_config.h"
#include "net/tcp_stream.h"
#include "openssl/ssl.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

class TlsStream;
using TlsStreamSptr = std::shared_ptr<TlsStream>;

class TlsStream  // noncopyable
{
public:
    // factory
    static TlsStreamSptr AsServer(SslConfig& config, TcpStreamSptr stream);
    static TlsStreamSptr AsClient(SslConfig& config, TcpStreamSptr stream);

    TlsStream(TcpStreamSptr stream);
    ~TlsStream();
    TlsStream(const TlsStream&) = delete;
    TlsStream& operator=(const TlsStream&) = delete;

    int AsyncRead(void* buf, int len);
    int AsyncRead(void* buf, int len, int ms, bool& timeout);
    int AsyncWrite(const void* buf, int count);

    // getter
    TcpStreamSptr stream() { return stream_; }

private:
    TcpStreamSptr stream_;

    SSL_CTX* ctx_;
    SSL* ssl_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_TLS_STREAM_H_