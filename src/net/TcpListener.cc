#include "net/TcpListener.h"

#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/SocketOps.h"

using namespace baize;

class net::TcpListener::Impl
{
public:
    Impl(uint16_t port)
      : addr_(port),
        sock_(sockets::creatTcpSocket(addr_.getFamily()))
    {
        sock_.bindAddress(addr_);
        sock_.listen();
    }

    int accept(InetAddress* peeraddr)
    {
        return sock_.accept(peeraddr);
    }

public:
    InetAddress addr_;
    Socket sock_;
};

net::TcpListener::TcpListener(uint16_t port)
  : impl_(std::make_unique<Impl>(port))
{
}

net::TcpListener::~TcpListener()
{
}

int net::TcpListener::accept(InetAddress* peeraddr)
{
    return impl_->accept(peeraddr);
}