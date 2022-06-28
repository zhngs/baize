#include "net/TcpStream.h"

#include "net/InetAddress.h"
#include "net/Socket.h"

using namespace baize;

class net::TcpStream::Impl
{
public:
    Impl(int fd, InetAddress peeraddr)
      : conn_(fd),
        peeraddr_(peeraddr)
    {
    }

    ssize_t read(void* buf, size_t count) { return conn_.read(buf, count); }
    ssize_t write(const void* buf, size_t count) { return conn_.write(buf, count); }

    int getSockfd() { return conn_.getSockfd(); }
    string getPeerIp() { return peeraddr_.getIp(); }
    string getPeerIpPort() { return peeraddr_.getIpPort(); }
    uint16_t getPeerPort() { return peeraddr_.getPort(); }

public:
    Socket conn_;
    InetAddress peeraddr_;
};

net::TcpStream::TcpStream(int fd, InetAddress peeraddr)
  : impl_(std::make_unique<Impl>(fd, peeraddr))
{
}

net::TcpStream::~TcpStream()
{
}

ssize_t net::TcpStream::read(void* buf, size_t count)
{
    return impl_->read(buf, count);
}

ssize_t net::TcpStream::write(const void* buf, size_t count)
{
    return impl_->write(buf, count);
}

int net::TcpStream::getSockfd()
{
    return impl_->getSockfd();
}

string net::TcpStream::getPeerIp()
{
    return impl_->getPeerIp();
}

string net::TcpStream::getPeerIpPort()
{
    return impl_->getPeerIpPort();
}