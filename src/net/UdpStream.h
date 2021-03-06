#ifndef BAIZE_UDPSTREAM_H
#define BAIZE_UDPSTREAM_H

#include <memory>

#include "net/InetAddress.h"
#include "net/NetType.h"
#include "net/Socket.h"

namespace baize
{

namespace net
{

class UdpStream  // noncopyable
{
public:
    // for client
    UdpStream();
    // for server
    explicit UdpStream(uint16_t port);
    ~UdpStream();
    UdpStream(const UdpStream&) = delete;
    UdpStream& operator=(const UdpStream&) = delete;

    static UdpStreamSptr asServer(uint16_t port);
    static UdpStreamSptr asClient();

    int sendto(const void* buf, int len, const InetAddress& addr);
    int recvfrom(void* buf, int len, InetAddress* addr);

    int asyncSendto(const void* buf, int len, const InetAddress& addr);
    int asyncRecvfrom(void* buf, int len, InetAddress* addr);

    InetAddress getLocalAddr();

private:
    InetAddress bindaddr_;
    std::unique_ptr<Socket> conn_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_UDPSTREAM_H