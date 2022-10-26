#ifndef BAIZE_TCPSTREAM_H_
#define BAIZE_TCPSTREAM_H_

#include <memory>

#include "net/inet_address.h"
#include "net/net_buffer.h"
#include "net/socket.h"
#include "runtime/event_loop.h"
#include "util/types.h"

namespace baize
{

namespace net
{

class TcpStream;
using TcpStreamSptr = std::shared_ptr<TcpStream>;

class TcpStream  // noncopyable
{
public:
    // factory function
    static TcpStreamSptr AsyncConnect(const char* ip, uint16_t port);

    TcpStream(int fd, InetAddress peeraddr);
    ~TcpStream();
    TcpStream(const TcpStream&) = delete;
    TcpStream& operator=(const TcpStream&) = delete;

    int AsyncRead(void* buf, int count);
    int AsyncRead(void* buf, int count, int ms, bool& timeout);

    int AsyncRead(Buffer& buf);
    int AsyncRead(Buffer& buf, int ms, bool& timeout);

    int AsyncWrite(const void* buf, int count);

    void ShutdownWrite();

    // getter
    int sockfd();
    string peer_ip_port();
    runtime::AsyncPark& async_park() { return async_park_; }

    // setter
    void set_tcp_nodelay();

private:
    SocketUptr conn_;
    InetAddress peeraddr_;
    runtime::AsyncPark async_park_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_TCPSTREAM_H