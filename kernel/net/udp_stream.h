#ifndef BAIZE_UDPSTREAM_H
#define BAIZE_UDPSTREAM_H

#include <memory>

#include "net/inet_address.h"
#include "net/socket.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace net
{

class UdpStream;
using UdpStreamSptr = std::shared_ptr<UdpStream>;

class UdpStream  // noncopyable
{
public:
    // factory function
    static UdpStreamSptr AsServer(uint16_t port);
    static UdpStreamSptr AsClient();

    // for client
    UdpStream(bool ipv6 = false);
    // for server
    explicit UdpStream(uint16_t port);
    ~UdpStream();
    UdpStream(const UdpStream&) = delete;
    UdpStream& operator=(const UdpStream&) = delete;

    int AsyncSendto(const void* buf, int len, const InetAddress& addr);
    int AsyncRecvFrom(void* buf, int len, InetAddress* addr);
    int AsyncRecvFrom(
        void* buf, int count, InetAddress* addr, int ms, bool& timeout);

    // getter
    InetAddress localaddr();
    int sockfd() { return conn_->sockfd(); }
    SocketUptr& socket() { return conn_; }

private:
    InetAddress bindaddr_;
    SocketUptr conn_;
    runtime::AsyncPark async_park_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_UDPSTREAM_H