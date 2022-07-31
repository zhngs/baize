#include "net/socket.h"

#include "arpa/inet.h"
#include "log/logger.h"
#include "net/inet_address.h"
#include "netinet/tcp.h"
#include "unistd.h"

namespace baize
{

namespace net
{

Socket::~Socket()
{
    if (::close(sockfd_) < 0) {
        LOG_SYSERR << "Socket::close";
    }
}

int Socket::Connect(const InetAddress& peeraddr)
{
    // todo: now is too simple
    return ::connect(sockfd_,
                     peeraddr.sockaddr(),
                     static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void Socket::BindAddress(const InetAddress& localaddr)
{
    int ret = ::bind(sockfd_,
                     localaddr.sockaddr(),
                     static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        LOG_SYSFATAL << "Socket::bindAddress";
    }
}

void Socket::Listen()
{
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0) {
        LOG_SYSFATAL << "Socket::listen";
    }
}

int Socket::Accept(InetAddress* peeraddr)
{
    struct sockaddr_in6 addr;
    memZero(&addr, sizeof(addr));

    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    int connfd = ::accept4(sockfd_,
                           reinterpret_cast<struct sockaddr*>(&addr),
                           &addrlen,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        int savedErrno = errno;
        LOG_SYSERR << "Socket::accept";
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:  // ???
            case EPERM:
            case EMFILE:  // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
        }
    }

    if (connfd >= 0) {
        peeraddr->set_sockaddr_in6(addr);
    }
    return connfd;
}

void Socket::ShutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_SYSERR << "sockets::ShutdownWrite";
    }
}

ssize_t Socket::Read(void* buf, size_t count)
{
    return ::read(sockfd_, buf, count);
}

ssize_t Socket::Write(const void* buf, size_t count)
{
    return ::write(sockfd_, buf, count);
}

ssize_t Socket::SendTo(const void* buf, size_t count, const InetAddress& addr)
{
    return ::sendto(
        sockfd_, buf, count, 0, addr.sockaddr(), sizeof(sockaddr_in6));
}

ssize_t Socket::RecvFrom(void* buf, size_t count, InetAddress* addr)
{
    sockaddr_in6 addr6;
    memZero(&addr6, sizeof(addr6));
    socklen_t len = sizeof(addr6);
    ssize_t rn = ::recvfrom(
        sockfd_, buf, count, 0, reinterpret_cast<sockaddr*>(&addr6), &len);
    if (rn > 0) {
        addr->set_sockaddr_in6(addr6);
    }
    return rn;
}

void Socket::set_tcp_nodelay(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_,
                           IPPROTO_TCP,
                           TCP_NODELAY,
                           &optval,
                           static_cast<socklen_t>(sizeof(optval)));
    if (ret < 0) {
        LOG_SYSERR << "set_tcp_nodelay to " << on << " failed";
    }
}

void Socket::set_reuse_addr(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_,
                           SOL_SOCKET,
                           SO_REUSEADDR,
                           &optval,
                           static_cast<socklen_t>(sizeof(optval)));
    if (ret < 0) {
        LOG_SYSERR << "set_reuse_addr to " << on << " failed";
    }
}

void Socket::set_reuse_port(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_,
                           SOL_SOCKET,
                           SO_REUSEPORT,
                           &optval,
                           static_cast<socklen_t>(sizeof(optval)));
    if (ret < 0) {
        LOG_SYSERR << "set_reuse_port to " << on << " failed";
    }
}

void Socket::set_keep_alive(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_,
                           SOL_SOCKET,
                           SO_KEEPALIVE,
                           &optval,
                           static_cast<socklen_t>(sizeof(optval)));
    if (ret < 0) {
        LOG_SYSERR << "set_keep_alive to " << on << " failed";
    }
}

InetAddress Socket::localaddr()
{
    struct sockaddr_in6 localaddr;
    memZero(&localaddr, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    if (::getsockname(sockfd_,
                      reinterpret_cast<struct sockaddr*>(&localaddr),
                      &addrlen) < 0) {
        LOG_SYSERR << "sockets::localaddr";
    }
    return InetAddress(localaddr);
}

InetAddress Socket::peeraddr()
{
    struct sockaddr_in6 peeraddr;
    memZero(&peeraddr, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));
    if (::getpeername(sockfd_,
                      reinterpret_cast<struct sockaddr*>(&peeraddr),
                      &addrlen) < 0) {
        LOG_SYSERR << "sockets::peeraddr";
    }
    return InetAddress(peeraddr);
}

int Socket::socket_error()
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if (::getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

bool Socket::is_self_connect()
{
    InetAddress laddr = localaddr();
    InetAddress paddr = peeraddr();
    const struct sockaddr* localaddr = laddr.sockaddr();
    const struct sockaddr* peeraddr = paddr.sockaddr();
    if (localaddr->sa_family == AF_INET) {
        const struct sockaddr_in* laddr4 =
            reinterpret_cast<const struct sockaddr_in*>(localaddr);
        const struct sockaddr_in* raddr4 =
            reinterpret_cast<const struct sockaddr_in*>(peeraddr);
        return laddr4->sin_port == raddr4->sin_port &&
               laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    } else if (localaddr->sa_family == AF_INET6) {
        const struct sockaddr_in6* laddr6 =
            reinterpret_cast<const struct sockaddr_in6*>(localaddr);
        const struct sockaddr_in6* raddr6 =
            reinterpret_cast<const struct sockaddr_in6*>(peeraddr);
        return laddr6->sin6_port == raddr6->sin6_port &&
               memcmp(&laddr6->sin6_addr,
                      &raddr6->sin6_addr,
                      sizeof(laddr6->sin6_addr)) == 0;
    } else {
        return false;
    }
}

int CreatTcpSocket(sa_family_t family)
{
    int sockfd = ::socket(
        family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_SYSFATAL << "sockets::CreatTcpSocket failed";
    }
    return sockfd;
}

int CreatUdpSocket(sa_family_t family)
{
    int sockfd = ::socket(
        family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
    if (sockfd < 0) {
        LOG_SYSFATAL << "sockets::CreatUdpSocket failed";
    }
    return sockfd;
}

}  // namespace net

}  // namespace baize