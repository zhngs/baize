#include "net/TcpListener.h"

#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/TcpStream.h"

using namespace baize;

class net::TcpListener::Impl
{
public:
    Impl(uint16_t port)
      : addr_(port),
        sock_(creatTcpSocket(addr_.getFamily()))
    {
        sock_.bindAddress(addr_);
        sock_.listen();
    }

    TcpStreamSptr accept()
    {
        InetAddress peeraddr;
        int connfd = sock_.accept(&peeraddr);
        if (connfd < 0) {
            return TcpStreamSptr();
        } else {
            return std::make_shared<TcpStream>(connfd, peeraddr);
        }
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

net::TcpStreamSptr net::TcpListener::accept()
{
    return impl_->accept();
}