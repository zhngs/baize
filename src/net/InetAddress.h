#ifndef BAIZE_INETADDRESS_H
#define BAIZE_INETADDRESS_H
// copy from muduo and make some small changes

#include <netinet/in.h>

#include "util/types.h"

namespace baize
{

namespace net
{

class InetAddress  // copyable
{
public:
    InetAddress() { memZero(&addr6_, sizeof(addr6_)); }
    InetAddress(const char* ip, uint16_t port, bool ipv6 = false);
    explicit InetAddress(uint16_t port,
                         bool loopback = false,
                         bool ipv6 = false);
    explicit InetAddress(struct sockaddr_in addr) : addr_(addr) {}
    explicit InetAddress(struct sockaddr_in6 addr) : addr6_(addr) {}
    explicit InetAddress(struct sockaddr_storage addr) : addrs_(addr) {}

    sa_family_t getFamily() const { return addr_.sin_family; }
    string getIp() const;
    string getIpPort() const;
    uint16_t getPort() const { return networkToHost16(addr_.sin_port); };
    uint16_t getPortNetEndian() const { return addr_.sin_port; }
    const struct sockaddr* getSockAddr() const
    {
        return reinterpret_cast<const struct sockaddr*>(&addr6_);
    }
    struct sockaddr* getSockAddr()
    {
        return reinterpret_cast<struct sockaddr*>(&addr6_);
    }
    socklen_t getSockLen() const;
    void setSockAddrIn6(const struct sockaddr_in6& addr) { addr6_ = addr; }

private:
    union {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
        struct sockaddr_storage addrs_;
    };
};

struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);

}  // namespace net

}  // namespace baize

#endif  // BAIZE_INETADDRESS_H