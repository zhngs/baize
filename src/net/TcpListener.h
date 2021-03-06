#ifndef BAIZE_TCPLISTENER_H
#define BAIZE_TCPLISTENER_H

#include <memory>

#include "net/InetAddress.h"
#include "net/NetType.h"
#include "net/Socket.h"

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

    void start();
    TcpStreamSptr accept();
    TcpStreamSptr asyncAccept();
    int getSockfd();

private:
    bool started_;
    InetAddress listenaddr_;
    std::unique_ptr<Socket> sock_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_TCPLISTENER_H