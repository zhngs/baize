#include "net/InetAddress.h"

#include <arpa/inet.h>

#include "log/Logger.h"

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

net::InetAddress::InetAddress(const char *ip, uint16_t port, bool ipv6) {
  if (ipv6 || strchr(ip, ':')) {
    memZero(&addr6_, sizeof(addr6_));
    addr6_.sin6_family = AF_INET6;
    addr6_.sin6_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET6, ip, &addr6_.sin6_addr) <= 0) {
      LOG_SYSERR << "inet_pton failed";
    }
  } else {
    memZero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr_.sin_addr) <= 0) {
      LOG_SYSERR << "inet_pton failed";
    }
  }
}

net::InetAddress::InetAddress(uint16_t port, bool loopback, bool ipv6) {
  if (ipv6) {
    memZero(&addr6_, sizeof(addr6_));
    addr6_.sin6_family = AF_INET6;
    in6_addr ip = loopback ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = hostToNetwork16(port);
  } else {
    memZero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopback ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr.s_addr = hostToNetwork32(ip);
    addr_.sin_port = hostToNetwork16(port);
  }
}

string net::InetAddress::getIpPort() const {
  char buf[64] = "";

  if (addr_.sin_family == AF_INET6) {
    snprintf(buf, sizeof(buf), "[%s]:%u", getIp().data(), getPort());
  } else if (addr_.sin_family == AF_INET) {
    snprintf(buf, sizeof(buf), "%s:%u", getIp().data(), getPort());
  }
  return buf;
}

string net::InetAddress::getIp() const {
  char buf[64] = "";
  if (addr_.sin_family == AF_INET) {
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf,
                static_cast<socklen_t>(sizeof(buf)));
  } else if (addr_.sin_family == AF_INET6) {
    ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf,
                static_cast<socklen_t>(sizeof(buf)));
  }
  return buf;
}

socklen_t net::InetAddress::getSockLen() const {
  if (addr_.sin_family == AF_INET) {
    return sizeof(addr_);
  } else if (addr_.sin_family == AF_INET6) {
    return sizeof(addr6_);
  } else {
    return -1;
  }
}
