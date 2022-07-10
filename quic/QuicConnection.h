#ifndef BAIZE_QUICCONNECTION_H
#define BAIZE_QUICCONNECTION_H

#include "net/InetAddress.h"
#include "net/NetType.h"
#include "QuicType.h"

namespace baize
{

namespace net
{

class QuicConfig;

class QuicConnection //noncopyable
{
public:
    QuicConnection(UdpStreamSptr udpstream,
                   InetAddress localaddr,
                   InetAddress peeraddr,
                   QuicConfigUptr config,
                   quiche_conn* conn);
    ~QuicConnection();
    QuicConnection(const QuicConnection&) = delete;
    QuicConnection& operator=(const QuicConnection&) = delete;

    static QuicConnSptr connect(const char* ip, uint16_t port);

    int quicStreamWrite(uint64_t streamid, const void* buf, int len, bool fin);
    int quicStreamRead(uint64_t streamid, void* buf, int len, bool* fin);

    bool fillQuic();
    bool isClosed();
private:
    bool flushQuic();
    bool untilEstablished();

    UdpStreamSptr udpstream_;
    InetAddress localaddr_;
    InetAddress peeraddr_;
    QuicConfigUptr config_;
    quiche_conn* conn_;
};

} // namespace quic

    
} // namespace baize


#endif //BAIZE_QUICCONNECTION_H