#ifndef BAIZE_HTTPS_SERVER_H_
#define BAIZE_HTTPS_SERVER_H_

#include "crypto/tls_stream.h"
#include "http/http_message.h"
#include "net/tcp_listener.h"

namespace baize
{

namespace net
{

class HttpsStream;
using HttpsStreamSptr = std::shared_ptr<HttpsStream>;

class HttpsListener  // noncopyable
{
public:
    explicit HttpsListener(uint16_t port);
    ~HttpsListener();
    HttpsListener(const HttpsListener&) = delete;
    HttpsListener& operator=(const HttpsListener&) = delete;

    HttpsStreamSptr AsyncAccept();

    // setter
    int set_cert_key(string cert, string key);

private:
    TcpListener listener_;
    SslConfig config_;
};

class HttpsStream  // noncopyable
{
public:
    HttpsStream(SslConfig& config, TcpStreamSptr stream);
    ~HttpsStream();
    HttpsStream(const HttpsStream&) = delete;
    HttpsStream& operator=(const HttpsStream&) = delete;

    int TlsHandshake();

    int AsyncRead(HttpRequest& req);
    int AsyncWrite(HttpResponseBuilder& rsp);

private:
    Buffer read_buf_;
    TcpStreamSptr tcp_stream_;
    SslConfig& config_;
    TlsStreamSptr tls_stream_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_HTTP_SERVER_H_
