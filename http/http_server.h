#ifndef BAIZE_HTTP_SERVER_H_
#define BAIZE_HTTP_SERVER_H_

#include "http/http_message.h"
#include "net/tcp_listener.h"

namespace baize
{

namespace net
{

using HttpHandler = std::function<void(const HttpRequest&, HttpResponse&)>;

class HttpServer  // noncopyable
{
public:
    HttpServer(uint16_t port, HttpHandler handler);
    ~HttpServer();
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    void Start();

private:
    void TcpLayer(TcpStreamSptr stream);
    int HttpLayer(Buffer& read_buf, Buffer& write_buf);

    TcpListener listener_;
    HttpHandler handler_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_HTTP_SERVER_H_
