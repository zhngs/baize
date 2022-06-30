#ifndef BAIZE_TCPSTREAM_H
#define BAIZE_TCPSTREAM_H

#include "net/InetAddress.h"
#include "util/types.h"

#include <memory>

namespace baize
{

namespace net
{

class Socket;

class TcpStream //noncopyable
{
public:
    TcpStream(int fd, InetAddress peeraddr);

    TcpStream(const TcpStream&) = delete;
    TcpStream& operator=(const TcpStream&) = delete;

    ssize_t read(void* buf, size_t count);
    ssize_t write(const void* buf, size_t count);

    int getSockfd();
    string getPeerIp();
    string getPeerIpPort();
private:
    std::unique_ptr<Socket> conn_;
    InetAddress peeraddr_;
};

    
} // namespace net
    
} // namespace baize


#endif //BAIZE_TCPSTREAM_H