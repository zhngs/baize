#ifndef BAIZE_QUICCONNECTION_H
#define BAIZE_QUICCONNECTION_H

#include "QuicType.h"
#include "net/NetType.h"
#include "net/inet_address.h"

namespace baize
{

namespace net
{

class QuicConfig;
extern thread_local uint8_t quicWriteBuffer[kMaxDatagramSize];
extern thread_local uint8_t quicReadBuffer[65536];

class QuicConnection  // noncopyable
{
public:
    QuicConnection(UdpStreamSptr udpstream,
                   InetAddress localaddr,
                   InetAddress peeraddr,
                   QuicConfigSptr config,
                   quiche_conn* conn);
    ~QuicConnection();
    QuicConnection(const QuicConnection&) = delete;
    QuicConnection& operator=(const QuicConnection&) = delete;

    static QuicConnSptr connect(const char* ip, uint16_t port);

    int quicStreamWrite(uint64_t streamid, const void* buf, int len, bool fin);
    int quicStreamRead(uint64_t streamid, void* buf, int len, bool* fin);

    void quicConnRead(void* buf, int len, InetAddress& peeraddr);

    bool fillQuic();
    bool isClosed();

private:
    bool flushQuic();
    bool untilEstablished();

    UdpStreamSptr udpstream_;
    InetAddress localaddr_;
    InetAddress peeraddr_;

    QuicConfigSptr config_;
    quiche_conn* conn_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_QUICCONNECTION_H