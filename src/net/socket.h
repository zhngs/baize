#ifndef BAIZE_SOCKET_H
#define BAIZE_SOCKET_H
// copy from muduo and make some small changes

#include <netinet/in.h>

#include "net/inet_address.h"
#include "util/types.h"

namespace baize
{

namespace net
{

int CreatTcpSocket(sa_family_t family);
int CreatUdpSocket(sa_family_t family);

class Socket  // noncopyable
{
public:
    explicit Socket(int fd) : sockfd_(fd) {}
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    int Connect(const InetAddress& peeraddr);
    void BindAddress(const InetAddress& localaddr);
    void Listen();
    int Accept(InetAddress* peeraddr);
    void ShutdownWrite();

    ssize_t Read(void* buf, size_t count);
    ssize_t Write(const void* buf, size_t count);
    ssize_t SendTo(const void* buf, size_t count, const InetAddress& addr);
    ssize_t RecvFrom(void* buf, size_t count, InetAddress* addr);

    // getter
    int sockfd() const { return sockfd_; }
    InetAddress localaddr();
    InetAddress peeraddr();
    int socket_error();
    bool is_self_connect();

    // setter
    void set_tcp_nodelay(bool on);
    void set_reuse_addr(bool on);
    void set_reuse_port(bool on);
    void set_keep_alive(bool on);

private:
    const int sockfd_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_SOCKET_H