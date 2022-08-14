#ifndef BAIZE_TCPSTREAM_H_
#define BAIZE_TCPSTREAM_H_

#include <memory>

#include "net/inet_address.h"
#include "net/net_buffer.h"
#include "net/socket.h"
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

    ssize_t Read(void* buf, size_t count);
    ssize_t Write(const void* buf, size_t count);

    int AsyncRead(void* buf, size_t count);
    int AsyncRead(void* buf, size_t count, double ms, bool& timeout);

    int AsyncRead();
    int AsyncRead(double ms, bool& timeout);

    // 返回值不等于count，说明出现异常错误
    int AsyncWrite(const void* buf, size_t count);

    void ShutdownWrite();

    // getter
    int sockfd();
    string peer_ip_port();
    Buffer* read_buffer();

    // setter
    void set_tcp_nodelay();

private:
    std::unique_ptr<Socket> conn_;
    InetAddress peeraddr_;
    Buffer read_buffer_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_TCPSTREAM_H