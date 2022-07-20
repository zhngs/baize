#ifndef BAIZE_TCPSTREAM_H
#define BAIZE_TCPSTREAM_H

#include <memory>

#include "net/InetAddress.h"
#include "net/NetType.h"
#include "util/types.h"

namespace baize
{

namespace runtime
{
class EventLoop;
}  // namespace runtime

namespace net
{

class Socket;

class TcpStream  // noncopyable
{
public:
    TcpStream(int fd, InetAddress peeraddr);
    ~TcpStream();

    TcpStream(const TcpStream&) = delete;
    TcpStream& operator=(const TcpStream&) = delete;

    ssize_t read(void* buf, size_t count);
    ssize_t write(const void* buf, size_t count);

    void shutdownWrite();
    void setTcpNoDelay();

    static TcpStreamSptr asyncConnect(const char* ip, uint16_t port);

    int asyncRead(void* buf, size_t count);
    int asyncWrite(const void* buf, size_t count);

    int asyncReadOrDie(void* buf, size_t count);
    int asyncWriteOrDie(const void* buf, size_t count);

    int getSockfd();
    string getPeerIp();
    string getPeerIpPort();

private:
    runtime::EventLoop* loop_;

    std::unique_ptr<Socket> conn_;
    InetAddress peeraddr_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_TCPSTREAM_H