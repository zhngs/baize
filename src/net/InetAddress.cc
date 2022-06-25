#include "net/InetAddress.h"

#include "net/SocketOps.h"

using namespace baize;

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

net::InetAddress::InetAddress(const char* ip, uint16_t port, bool ipv6)
{
    if (ipv6 || strchr(ip, ':')) {
        memZero(&addr6_, sizeof addr6_);
        sockets::fromIpPort(ip, port, &addr6_);
    } else {
        memZero(&addr_, sizeof addr_);
        sockets::fromIpPort(ip, port, &addr_);
    }
}

net::InetAddress::InetAddress(uint16_t port, bool loopback, bool ipv6)
{
    if (ipv6) {
        memZero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopback ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = sockets::hostToNetwork16(port);
    } else {
        memZero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopback ? INADDR_LOOPBACK : INADDR_ANY;
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
        addr_.sin_port = sockets::hostToNetwork16(port);
    }
}

string net::InetAddress::getIpPort() const
{
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof(buf), getSockAddr());
    return buf;
}

string net::InetAddress::getIp() const
{
    char buf[64] = "";
    sockets::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

uint16_t net::InetAddress::getPort() const
{
    return sockets::networkToHost16(getPortNetEndian());
}