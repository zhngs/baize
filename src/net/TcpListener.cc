#include "net/TcpListener.h"

#include "net/TcpStream.h"
#include "runtime/EventLoop.h"

using namespace baize;

net::TcpListener::TcpListener(uint16_t port)
  : started_(false),
    listenaddr_(port),
    sock_(std::make_unique<Socket>(creatTcpSocket((listenaddr_.getFamily()))))
{
    sock_->setReuseAddr(true);
}

net::TcpListener::~TcpListener()
{
    if (started_) {
        runtime::EventLoop* loop = runtime::getCurrentLoop();
        loop->unregisterPollEvent(sock_->getSockfd());
    }
}

void net::TcpListener::start()
{
    started_ = true;
    sock_->bindAddress(listenaddr_);
    sock_->listen();
    runtime::EventLoop* loop = runtime::getCurrentLoop();
    loop->registerPollEvent(sock_->getSockfd());
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
    runtime::EventLoop* loop = runtime::getCurrentLoop();
    InetAddress peeraddr;
    while (1) {
        loop->checkRoutineTimeout();
        int connfd = sock_->accept(&peeraddr);
        if (connfd < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                loop->addWaitRequest(sock_->getSockfd(),
                                     WAIT_READ_REQUEST,
                                     runtime::getCurrentRoutineId());
                loop->backToMainRoutine();
                continue;
            }
        } else {
            return std::make_shared<TcpStream>(connfd, peeraddr);
        }
    }
}

int net::TcpListener::getSockfd() { return sock_->getSockfd(); }