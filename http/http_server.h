#ifndef BAIZE_HTTP_SERVER_H_
#define BAIZE_HTTP_SERVER_H_

#include "http/http_message.h"
#include "net/tcp_listener.h"

namespace baize
{

namespace net
{

class HttpStream;
using HttpStreamSptr = std::shared_ptr<HttpStream>;

class HttpListener  // noncopyable
{
public:
    explicit HttpListener(uint16_t port);
    ~HttpListener();
    HttpListener(const HttpListener&) = delete;
    HttpListener& operator=(const HttpListener&) = delete;

    HttpStreamSptr AsyncAccept();

private:
    TcpListener listener_;
};

class HttpStream  // noncopyable
{
public:
    explicit HttpStream(TcpStreamSptr stream);
    ~HttpStream();
    HttpStream(const HttpStream&) = delete;
    HttpStream& operator=(const HttpStream&) = delete;

    int AsyncRead(HttpRequest& req);
    int AsyncWrite(HttpResponseBuilder& rsp);

private:
    Buffer read_buf_;
    TcpStreamSptr stream_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_HTTP_SERVER_H_
