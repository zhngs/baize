#ifndef BAIZE_INETADDRESS_H_
#define BAIZE_INETADDRESS_H_
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
    InetAddress() { MemZero(&addr6_, sizeof(addr6_)); }
    InetAddress(const char* ip, uint16_t port, bool ipv6 = false);
    explicit InetAddress(uint16_t port,
                         bool loopback = false,
                         bool ipv6 = false);
    explicit InetAddress(struct sockaddr_in addr) : addr_(addr) {}
    explicit InetAddress(struct sockaddr_in6 addr) : addr6_(addr) {}
    explicit InetAddress(struct sockaddr_storage addr) : addrs_(addr) {}

    // getter
    sa_family_t family() const { return addr_.sin_family; }
    string ip() const;
    string ip_port() const;
    uint16_t port() const { return networkToHost16(addr_.sin_port); };
    const struct sockaddr* sockaddr() const
    {
        return reinterpret_cast<const struct sockaddr*>(&addr6_);
    }
    struct sockaddr* sockaddr()
    {
        return reinterpret_cast<struct sockaddr*>(&addr6_);
    }
    socklen_t socklen() const;

    // setter
    void set_sockaddr_in6(const struct sockaddr_in6& addr) { addr6_ = addr; }

private:
    union {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
        struct sockaddr_storage addrs_;
    };
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_INETADDRESS_H