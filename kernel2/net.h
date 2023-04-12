#ifndef BAIZE_NET_H
#define BAIZE_NET_H

#include "io.h"

namespace baize {

class IAddr {
 public:
  virtual ~IAddr() {}
  virtual string Network() = 0;
  virtual string String() = 0;
};
using Addr = shared_ptr<IAddr>;

class IConn : public IReader, public IWriter {
 public:
  virtual ~IConn() {}
  virtual Addr LocalAddr() = 0;
  virtual Addr RemoteAddr() = 0;
  virtual void SetReadDeadline(int ms) = 0;
  virtual void SetWriteDeadline(int ms) = 0;
};
using Conn = shared_ptr<IConn>;

class IListener {
 public:
  virtual ~IListener() {}
  virtual Conn Accept() = 0;
  virtual Addr Addr() = 0;
};
using Listener = shared_ptr<IListener>;

class IPacketConn {
 public:
  virtual ~IPacketConn() {}
  virtual result<int> ReadFrom(slice<byte> p, Addr& addr) = 0;
  virtual result<int> WriteTo(slice<byte> p, Addr addr) = 0;
  virtual Addr LocalAddr() = 0;
  virtual Addr RemoteAddr() = 0;
  virtual void SetReadDeadline(int ms) = 0;
  virtual void SetWriteDeadline(int ms) = 0;
};
using PacketConn = shared_ptr<IPacketConn>;

result<Conn> DialTcp(string network, string address);

}  // namespace baize

#endif  // BAIZE_NET_H