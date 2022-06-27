#include "net/TcpStream.h"

#include "net/Socket.h"

using namespace baize;

class net::TcpStream::Impl
{
public:
    Impl(int fd)
      : conn_(fd)
    {
    }
public:
    Socket conn_;
};

net::TcpStream::TcpStream(int fd)
  : impl_(std::make_unique<Impl>(fd))
{
}

net::TcpStream::~TcpStream()
{
}