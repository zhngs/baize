#include "net/TcpListener.h"

#include "net/Socket.h"
#include "net/TcpStream.h"
#include "runtime/EventLoop.h"

using namespace baize;

net::TcpListener::TcpListener(uint16_t port)
  : started_(false),
    loop_(runtime::EventLoop::getCurrentLoop()),
    listenaddr_(port),
    sock_(std::make_unique<Socket>(creatTcpSocket((listenaddr_.getFamily()))))
{
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
        int connfd = sock_->accept(&peeraddr);
        if (connfd < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                loop_->addWaitRequest(sock_->getSockfd(), WAIT_READ_REQUEST, loop_->getCurrentRoutineId());
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