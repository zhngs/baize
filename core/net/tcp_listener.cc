#include "net/tcp_listener.h"

#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

TcpListener::TcpListener(uint16_t port)
  : started_(false),
    listenaddr_(port),
    sock_(std::make_unique<Socket>(CreatTcpSocket((listenaddr_.family()))))
{
    sock_->set_reuse_addr(true);
}

TcpListener::~TcpListener()
{
    if (started_) {
        runtime::EventLoop* loop = runtime::current_loop();
        loop->DisablePoll(sock_->sockfd());
    }
}

void TcpListener::Start()
{
    started_ = true;
    sock_->BindAddress(listenaddr_);
    sock_->Listen();
    runtime::EventLoop* loop = runtime::current_loop();
    loop->EnablePoll(sock_->sockfd());
}

TcpStreamSptr TcpListener::Accept()
{
    InetAddress peeraddr;
    int connfd = sock_->Accept(&peeraddr);
    if (connfd < 0) {
        return TcpStreamSptr();
    } else {
        return std::make_shared<TcpStream>(connfd, peeraddr);
    }
}

TcpStreamSptr TcpListener::AsyncAccept()
{
    runtime::EventLoop* loop = runtime::current_loop();
    InetAddress peeraddr;
    while (1) {
        loop->CheckTicks();
        int connfd = sock_->Accept(&peeraddr);
        if (connfd < 0) {
            if (errno == EAGAIN) {
                loop->WaitReadable(sock_->sockfd());
                runtime::Return();
                continue;
            }
        } else {
            return std::make_shared<TcpStream>(connfd, peeraddr);
        }
    }
}

TcpStreamSptr TcpListener::AsyncAccept(double ms, bool& timeout)
{
    runtime::EventLoop* loop = runtime::current_loop();
    InetAddress peeraddr;
    while (1) {
        loop->CheckTicks();
        int connfd = sock_->Accept(&peeraddr);
        if (connfd < 0) {
            if (errno == EAGAIN) {
                loop->WaitReadable(sock_->sockfd(), ms, timeout);
                runtime::Return();
                if (timeout) {
                    return TcpStreamSptr();
                }
                continue;
            }
        } else {
            return std::make_shared<TcpStream>(connfd, peeraddr);
        }
    }
}

int TcpListener::sockfd() { return sock_->sockfd(); }

}  // namespace net

}  // namespace baize