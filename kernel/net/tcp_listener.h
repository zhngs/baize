#ifndef BAIZE_TCPLISTENER_H_
#define BAIZE_TCPLISTENER_H_

#include <memory>

#include "net/inet_address.h"
#include "net/socket.h"
#include "net/tcp_stream.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

class TcpListener  // noncopyable
{
public:
    explicit TcpListener(uint16_t port);
    ~TcpListener();

    TcpListener(const TcpListener&) = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    void Start();
    TcpStreamSptr Accept();

    TcpStreamSptr AsyncAccept();
    TcpStreamSptr AsyncAccept(int ms);

    // getter
    int sockfd();

private:
    InetAddress listenaddr_;
    std::unique_ptr<Socket> sock_;
    runtime::AsyncPark async_park_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_TCPLISTENER_H