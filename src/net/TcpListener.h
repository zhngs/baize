#ifndef BAIZE_TCPLISTENER_H
#define BAIZE_TCPLISTENER_H

#include "util/noncopyable.h"

#include <memory>

namespace baize
{

namespace net
{

class Socket;
class TcpStream;
class InetAddress;

class TcpListener: noncopyable
{
public:
    explicit TcpListener(uint16_t port);
    ~TcpListener();

    int accept(InetAddress* peeraddr);
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
    
} // namespace net
    
} // namespace baize


#endif //BAIZE_TCPLISTENER_H