#ifndef BAIZE_NET_H
#define BAIZE_NET_H

#include "io.h"

namespace baize
{

class Addr {
public:
    virtual ~Addr() {}
    virtual string Network() = 0;
    virtual string String() = 0;
};

class Listener {
public:
    virtual ~Listener() {}
    virtual shared_ptr<Conn> Accept() = 0;
    virtual shared_ptr<Addr> Addr() = 0;
};

class Conn: public IReader, public IWriter {
public:
    virtual ~Conn() {}
    virtual shared_ptr<Addr> LocalAddr() = 0;
    virtual shared_ptr<Addr> RemoteAddr() = 0;
    virtual void SetReadDeadline(int ms) = 0;
    virtual void SetWriteDeadline(int ms) = 0;
};

class PacketConn {
public:
    virtual ~PacketConn() {}
    virtual result<int> ReadFrom(slice<byte> p, shared_ptr<Addr>& addr) = 0;
    virtual result<int> WriteTo(slice<byte> p, shared_ptr<Addr> addr) = 0;
    virtual shared_ptr<Addr> LocalAddr() = 0;
    virtual shared_ptr<Addr> RemoteAddr() = 0;
    virtual void SetReadDeadline(int ms) = 0;
    virtual void SetWriteDeadline(int ms) = 0;
};

} // namespace baize


#endif // BAIZE_NET_H