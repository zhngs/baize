#include "net/TcpListener.h"

#include "net/Socket.h"
#include "net/TcpStream.h"
#include "runtime/EventLoop.h"

using namespace baize;

net::TcpListener::TcpListener(uint16_t port)
  : started_(false),
    loop_(runtime::getCurrentLoop()),
    listenaddr_(port),
    sock_(std::make_unique<Socket>(creatTcpSocket((listenaddr_.getFamily()))))
{
    sock_->setReuseAddr(true);
}

net::TcpListener::~TcpListener()
{
    if (started_) {
        loop_->unregisterPollEvent(sock_->getSockfd());
    }
}

void net::TcpListener::start()
{
    started_ = true;
    sock_->bindAddress(listenaddr_);
    sock_->listen();
    loop_->registerPollEvent(sock_->getSockfd());
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

net::TcpStreamSptr net::TcpListener::asyncAccept()
{
    InetAddress peeraddr;
    while (1) {
        loop_->checkRoutineTimeout();
        int connfd = sock_->accept(&peeraddr);
        if (connfd < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                loop_->addWaitRequest(sock_->getSockfd(), WAIT_READ_REQUEST, runtime::getCurrentRoutineId());
                loop_->backToMainRoutine();
                continue;
            }
        } else {
            return std::make_shared<TcpStream>(connfd, peeraddr);
        }
    }
}

int net::TcpListener::getSockfd()
{
    return sock_->getSockfd();
}