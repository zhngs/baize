#ifndef BAIZE_TCPLISTENER_H
#define BAIZE_TCPLISTENER_H

#include "net/InetAddress.h"

#include <memory>

namespace baize
{

namespace net
{

class Socket;
class TcpStream;
using TcpStreamSptr = std::shared_ptr<TcpStream>;

class TcpListener //noncopyable
{
public:
    explicit TcpListener(uint16_t port);
    ~TcpListener();

    TcpListener(const TcpListener&) = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    void start();
    TcpStreamSptr accept();
    int getSockfd();
private:
    InetAddress listenaddr_;
    std::unique_ptr<Socket> sock_;
};
    
} // namespace net
    
} // namespace baize


#endif //BAIZE_TCPLISTENER_H