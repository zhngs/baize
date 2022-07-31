#ifndef BAIZE_QUICLISTENER_H
#define BAIZE_QUICLISTENER_H

#include <map>

#include "QuicType.h"
#include "net/NetType.h"
#include "net/inet_address.h"

namespace baize
{

namespace net
{

class QuicListenerData;

class QuicListener  // noncopyable
{
public:
    QuicListener(uint16_t port);
    ~QuicListener();
    QuicListener(const QuicListener&) = delete;
    QuicListener& operator=(const QuicListener&) = delete;

    void loopAndAccept();

private:
    bool quicNegotiate(QuicListenerData* data);
    bool validateToken(QuicListenerData* data);
    QuicConnSptr quicAccept(QuicListenerData* data);

    UdpStreamSptr udpstream_;
    InetAddress localaddr_;

    QuicConfigSptr config_;
    std::map<QuicConnId, QuicConnSptr> conns_;
};

}  // namespace net

}  // namespace baize

#endif  // BAIZE_QUICLISTENER_H