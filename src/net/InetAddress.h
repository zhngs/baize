#ifndef BAIZE_INETADDRESS_H
#define BAIZE_INETADDRESS_H

#include "util/noncopyable.h"
#include "util/types.h"

#include <netinet/in.h>

namespace baize
{
    
namespace net
{

class InetAddress: public util::copyable
{
public:
    InetAddress(const char* ip, uint16_t port, bool ipv6 = false);
    explicit InetAddress(uint16_t port, bool loopback = false, bool ipv6 = false);
    explicit InetAddress(struct sockaddr_in addr): addr_(addr) { }
    explicit InetAddress(struct sockaddr_in6 addr): addr6_(addr) { }

    sa_family_t getFamily() const { return addr_.sin_family; }
    string getIp() const;
    string getIpPort() const;
    uint16_t getPort() const;
    uint16_t getPortNetEndian() const { return addr_.sin_port; }
    const struct sockaddr* getSockAddr() const { return reinterpret_cast<const struct sockaddr*>(&addr6_); }
    void setSockaddrIn6(const struct sockaddr_in6& addr) { addr6_ = addr; }
private:
    union
    {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};
    
} // namespace net

} // namespace baize


#endif //BAIZE_INETADDRESS_H