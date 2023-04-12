#include "net.h"

#include <arpa/inet.h>
#include <netinet/in.h>

namespace baize {

inline uint64_t HostToNetwork64(uint64_t host64) { return htobe64(host64); }
inline uint32_t HostToNetwork32(uint32_t host32) { return htobe32(host32); }
inline uint16_t HostToNetwork16(uint16_t host16) { return htobe16(host16); }

inline uint64_t NetworkToHost64(uint64_t net64) { return be64toh(net64); }
inline uint32_t NetworkToHost32(uint32_t net32) { return be32toh(net32); }
inline uint16_t NetworkToHost16(uint16_t net16) { return be16toh(net16); }

class InetAddress : public copyable {
 public:
  InetAddress() { memset(&addr6_, 0, sizeof(addr6_)); }
  InetAddress(const char* ip, uint16_t port, bool ipv6 = false);
  explicit InetAddress(uint16_t port, bool loopback = false, bool ipv6 = false);
  explicit InetAddress(struct sockaddr_in addr) : addr_(addr) {}
  explicit InetAddress(struct sockaddr_in6 addr) : addr6_(addr) {}

  sa_family_t family() const { return addr_.sin_family; }
  string ip() const;
  string ip_port() const;
  uint16_t port() const { return NetworkToHost16(addr_.sin_port); };
  const struct sockaddr_in* sockaddr_in() const { return &addr_; }
  const struct sockaddr_in6* sockaddr_in6() const { return &addr6_; }
  const struct sockaddr* sockaddr() const {
    return reinterpret_cast<const struct sockaddr*>(&addr6_);
  }
  socklen_t socklen() const;

 private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

InetAddress::InetAddress(const char* ip, uint16_t port, bool ipv6) {
  if (ipv6 || strchr(ip, ':')) {
    memset(&addr6_, 0, sizeof(addr6_));
    addr6_.sin6_family = AF_INET6;
    addr6_.sin6_port = HostToNetwork16(port);
    if (::inet_pton(AF_INET6, ip, &addr6_.sin6_addr) <= 0) {
      //   LOG_SYSERR << "inet_pton failed";
    }
  } else {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = HostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr_.sin_addr) <= 0) {
      //   LOG_SYSERR << "inet_pton failed";
    }
  }
}

InetAddress::InetAddress(uint16_t port, bool loopback, bool ipv6) {
  if (ipv6) {
    memset(&addr6_, 0, sizeof(addr6_));
    addr6_.sin6_family = AF_INET6;
    in6_addr ip = loopback ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = HostToNetwork16(port);
  } else {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopback ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr.s_addr = HostToNetwork32(ip);
    addr_.sin_port = HostToNetwork16(port);
  }
}

string InetAddress::ip_port() const {
  char buf[64] = "";

  if (addr_.sin_family == AF_INET6) {
    snprintf(buf, sizeof(buf), "[%s]:%u", ip().data(), port());
  } else if (addr_.sin_family == AF_INET) {
    snprintf(buf, sizeof(buf), "%s:%u", ip().data(), port());
  }
  return buf;
}

string InetAddress::ip() const {
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

socklen_t InetAddress::socklen() const {
  if (addr_.sin_family == AF_INET) {
    return sizeof(addr_);
  } else if (addr_.sin_family == AF_INET6) {
    return sizeof(addr6_);
  } else {
    return -1;
  }
}

class TcpAddr : public IAddr, public copyable {
 public:
  TcpAddr(InetAddress addr) { addr_ = addr; }
  string Network() {
    if (addr_.family() == AF_INET) {
      return "tcp4";
    } else if (addr_.family() == AF_INET6) {
      return "tcp6";
    } else {
      return "unknown";
    }
  };
  string String() { return addr_.ip_port(); }

 private:
  InetAddress addr_;
};

result<Conn> DialTcp(string network, string address) {
  bool ipv6 = false;
  if (network == "tcp4") {
    ipv6 = false;
  } else if (network == "tcp6") {
    ipv6 = true;
  } else {
    return {-1, "network err"};
  }

  InetAddress addr;
}

}  // namespace baize
