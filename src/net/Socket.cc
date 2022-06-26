#include "net/Socket.h"

#include "log/Logger.h"
#include "net/InetAddress.h"
#include "net/SocketOps.h"

#include "netinet/tcp.h"

using namespace baize;

net::Socket::~Socket()
{
    sockets::close(sockfd_);
}

int net::Socket::connect(const InetAddress& peeraddr)
{
    return sockets::connect(sockfd_, peeraddr.getSockAddr());
}

void net::Socket::bindAddress(const InetAddress &localaddr)
{
    sockets::bindOrDie(sockfd_, localaddr.getSockAddr());
}

void net::Socket::listen()
{
    sockets::listenOrDie(sockfd_);
}

int net::Socket::accept(InetAddress *peeraddr)
{
    struct sockaddr_in6 addr;
    memZero(&addr, sizeof(addr));
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->setSockAddrIn6(addr);
    }
    return connfd;
}

void net::Socket::shutdownWrite()
{
    sockets::shutdownWrite(sockfd_);
}

void net::Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}

void net::Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}

void net::Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
}

void net::Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}