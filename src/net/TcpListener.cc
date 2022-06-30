#include "net/TcpListener.h"

#include "net/Socket.h"
#include "net/TcpStream.h"

using namespace baize;

net::TcpListener::TcpListener(uint16_t port)
  : listenaddr_(port),
    sock_(std::make_unique<Socket>(creatTcpSocket((listenaddr_.getFamily()))))
{
}

net::TcpListener::~TcpListener()
{
}

void net::TcpListener::start()
{
    sock_->bindAddress(listenaddr_);
    sock_->listen();
}

net::TcpStreamSptr net::TcpListener::accept()
{
    InetAddress peeraddr;
    int connfd = sock_->accept(&peeraddr);
    if (connfd < 0) {
        return TcpStreamSptr();
    } else {
        return std::make_shared<TcpStream>(connfd, peeraddr);
    }
}

int net::TcpListener::getSockfd()
{
    return sock_->getSockfd();
}