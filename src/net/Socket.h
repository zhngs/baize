#ifndef BAIZE_SOCKET_H
#define BAIZE_SOCKET_H
// copy from muduo and make some small changes

#include "util/noncopyable.h"

namespace baize
{

namespace net
{

class InetAddress;

class Socket: noncopyable
{
public:
    explicit Socket(int fd): sockfd_(fd) { }
    ~Socket();

    int getSockfd() { return sockfd_; }
    int connect(const InetAddress& peeraddr);
    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);
    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};
    
} // namespace net
    
} // namespace baize


#endif //BAIZE_SOCKET_H