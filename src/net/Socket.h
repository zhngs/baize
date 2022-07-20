#ifndef BAIZE_SOCKET_H
#define BAIZE_SOCKET_H
// copy from muduo and make some small changes

#include <netinet/in.h>

#include "util/types.h"

namespace baize
{

namespace net
{

class InetAddress;

class Socket  // noncopyable
{
public:
    explicit Socket(int fd) : sockfd_(fd) {}
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    int getSockfd() { return sockfd_; }
    int connect(const InetAddress& peeraddr);
    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);
    void shutdownWrite();

    ssize_t read(void* buf, size_t count);
    ssize_t write(const void* buf, size_t count);
    ssize_t sendto(const void* buf, size_t count, const InetAddress& addr);
    ssize_t recvfrom(void* buf, size_t count, InetAddress* addr);

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

    InetAddress getLocalAddr();
    InetAddress getPeerAddr();
    bool isSelfConnect();

    int getSocketError();
    int getSockfd() const { return sockfd_; }

private:
    const int sockfd_;
};

int creatTcpSocket(sa_family_t family);
int creatUdpSocket(sa_family_t family);

}  // namespace net

}  // namespace baize

#endif  // BAIZE_SOCKET_H