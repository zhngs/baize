#include "net/TcpStream.h"

#include "net/InetAddress.h"
#include "net/Socket.h"

using namespace baize;

net::TcpStream::TcpStream(int fd, InetAddress peeraddr)
  : conn_(std::make_unique<Socket>(fd)),
    peeraddr_(peeraddr)
{
}

ssize_t net::TcpStream::read(void* buf, size_t count)
{
    return conn_->read(buf, count);
}

ssize_t net::TcpStream::write(const void* buf, size_t count)
{
    return conn_->write(buf, count);
}

int net::TcpStream::getSockfd()
{
    return conn_->getSockfd();
}

string net::TcpStream::getPeerIpPort()
{
    return peeraddr_.getIpPort();
}