#include "net/tcp_listener.h"

#include "log/logger.h"

namespace baize
{

namespace net
{

TcpListener::TcpListener(uint16_t port)
  : listenaddr_(port),
    sock_(std::make_unique<Socket>(CreatTcpSocket((listenaddr_.family())))),
    async_park_(sock_->sockfd())
{
    sock_->set_reuse_addr(true);
}

TcpListener::~TcpListener() {}

void TcpListener::Start()
{
    sock_->BindAddress(listenaddr_);
    sock_->Listen();
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
    InetAddress peeraddr;
    while (1) {
        async_park_.CheckTicks();
        int connfd = sock_->Accept(&peeraddr);
        if (connfd < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                async_park_.WaitRead();
                continue;
            } else {
                LOG_ERROR << "AsyncAccept failed";
                return TcpStreamSptr();
            }
        } else {
            return std::make_shared<TcpStream>(connfd, peeraddr);
        }
    }
}

TcpStreamSptr TcpListener::AsyncAccept(int ms)
{
    InetAddress peeraddr;
    while (1) {
        async_park_.CheckTicks();
        int connfd = sock_->Accept(&peeraddr);
        if (connfd < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) {
                bool timeout = false;
                async_park_.WaitRead(ms, timeout);

                if (timeout) {
                    return TcpStreamSptr();
                } else {
                    continue;
                }
            } else {
                LOG_ERROR << "AsyncAccept failed";
                return TcpStreamSptr();
            }
        } else {
            return std::make_shared<TcpStream>(connfd, peeraddr);
        }
    }
}

int TcpListener::sockfd() { return sock_->sockfd(); }

}  // namespace net

}  // namespace baize