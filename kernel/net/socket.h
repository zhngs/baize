#ifndef BAIZE_SOCKET_H
#define BAIZE_SOCKET_H
// copy from muduo and make some small changes

#include <netinet/in.h>

#include <memory>

#include "net/inet_address.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class Socket;
using SocketUptr = std::unique_ptr<Socket>;

class Socket  // noncopyable
{
public:  // factory
    static SocketUptr NewTcp(sa_family_t family);
    static SocketUptr NewUdp(sa_family_t family);

public:  // special function
    explicit Socket(int fd) : sockfd_(fd) {}
    ~Socket();
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

public:  // normal function
    int Connect(const InetAddress& peeraddr);
    void BindAddress(const InetAddress& localaddr);
    void Listen();
    int Accept(InetAddress* peeraddr);
    void ShutdownWrite();
    void Close();

    int Read(void* buf, size_t count);
    int Write(const void* buf, size_t count);
    int SendTo(const void* buf, size_t count, const InetAddress& addr);
    int RecvFrom(void* buf, size_t count, InetAddress* addr);

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
    int sockfd_;
};

int CreateTcpSocket(sa_family_t family);
int CreateUdpSocket(sa_family_t family);

}  // namespace net

}  // namespace baize

#endif  // BAIZE_SOCKET_H