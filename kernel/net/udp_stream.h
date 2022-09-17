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
    UdpStream();
    // for server
    explicit UdpStream(uint16_t port);
    ~UdpStream();
    UdpStream(const UdpStream&) = delete;
    UdpStream& operator=(const UdpStream&) = delete;

    int SendTo(const void* buf, int len, const InetAddress& addr);
    int RecvFrom(void* buf, int len, InetAddress* addr);

    int AsyncSendto(const void* buf, int len, const InetAddress& addr);
    int AsyncRecvFrom(void* buf, int len, InetAddress* addr);
    // int AsyncRecvFrom(
    //     void* buf, int count, InetAddress* addr, double ms, bool& timeout);

    // getter
    InetAddress localaddr();

private:
    InetAddress bindaddr_;
    std::unique_ptr<Socket> conn_;
    runtime::AsyncPark async_park_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_UDPSTREAM_H