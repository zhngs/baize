#include "QuicConnection.h"

#include "net/UdpStream.h"
#include "QuicConfig.h"
#include "RandomFile.h"

#include <fcntl.h>

using namespace baize;

thread_local uint8_t quicWriteBuffer[net::kMaxDatagramSize];
thread_local uint8_t quicReadBuffer[65536];

net::QuicConnection::QuicConnection(UdpStreamSptr udpstream,
                                    InetAddress localaddr,
                                    InetAddress peeraddr,
                                    QuicConfigUptr config,
                                    quiche_conn* conn)
  : udpstream_(udpstream),
    localaddr_(localaddr),
    peeraddr_(peeraddr),
    config_(std::move(config)),
    conn_(conn)
{
}

net::QuicConnection::~QuicConnection()
{    
    quiche_conn_free(conn_);
}

net::QuicConnSptr net::QuicConnection::connect(const char* ip, uint16_t port)
{
    InetAddress peeraddr(ip, port);
    UdpStreamSptr udpstream(UdpStream::asClient());
    InetAddress localaddr = udpstream->getLocalAddr();
    QuicConfigUptr config(std::make_unique<QuicConfig>(0xbabababa));
    QuicConnId scid;
    RandomFile::getInstance().genRandom(scid.data(), scid.length());
    quiche_conn *conn = quiche_connect(ip, scid.data(), scid.length(),
                                       localaddr.getSockAddr(), localaddr.getSockLen(),
                                       peeraddr.getSockAddr(), peeraddr.getSockLen(), config->getConfig());
    if (conn == nullptr) {
        LOG_ERROR << "quiche_connect failed";
        return QuicConnSptr();
    }

    QuicConnSptr quic_conn(std::make_shared<QuicConnection>(udpstream, localaddr, peeraddr, std::move(config), conn));
    bool ret = quic_conn->flushQuic(); 
    if (!ret) {
        return QuicConnSptr();
    }

    ret = quic_conn->untilEstablished();
    if (!ret) {
        return QuicConnSptr();
    }

    return quic_conn;
}

bool net::QuicConnection::flushQuic()
{
    quiche_send_info send_info;
    while (1) {
        ssize_t written = quiche_conn_send(conn_, quicWriteBuffer, sizeof(quicWriteBuffer), &send_info);
        if (written == QUICHE_ERR_DONE) {
            break;
        }
        if (written < 0) {
            LOG_ERROR << "failed to create packet";
            return false;
        }

        ssize_t sent = udpstream_->asyncSendto(quicWriteBuffer, static_cast<int>(written), peeraddr_);
        if (sent != written) {
            LOG_SYSERR << "failed to send";
            return false;
        }
        LOG_INFO << "send " << written << " bytes";
    }
    return true;
}

bool net::QuicConnection::untilEstablished()
{
    while (1) {
        InetAddress peeraddr;
        ssize_t read = udpstream_->asyncRecvfrom(quicReadBuffer, sizeof(quicReadBuffer), &peeraddr);
        if (read < 0) {
            return false;
        }

        quiche_recv_info recv_info = {
            peeraddr.getSockAddr(),
            peeraddr.getSockLen(),
            localaddr_.getSockAddr(),
            localaddr_.getSockLen(),
        };
        ssize_t done = quiche_conn_recv(conn_, quicReadBuffer, read, &recv_info);
        if (done < 0) {
            continue;
        }

        LOG_INFO << "recv " << done << " bytes";


        if (quiche_conn_is_established(conn_)) {
            LOG_INFO << "quic conn established";
            break;
        }

        if (quiche_conn_is_closed(conn_)) {
            LOG_INFO << "quic conn closed";
            return false;
        }

        bool ret = flushQuic();
        if (!ret) {
            return false;
        }
    }
    return true;
}

int net::QuicConnection::quicStreamWrite(uint64_t streamid, const void* buf, int len, bool fin)
{
    ssize_t wn = quiche_conn_stream_send(conn_, streamid, static_cast<const uint8_t*>(buf), len, fin);
    if (wn <= 0) {
        return static_cast<int>(wn);
    }
    // should buffered
    assert(wn == len);

    bool ret = flushQuic();
    if (!ret) {
        return -1;
    }

    return static_cast<int>(wn);
}

int net::QuicConnection::quicStreamRead(uint64_t streamid, void* buf, int len, bool* fin)
{
    ssize_t rn = quiche_conn_stream_recv(conn_, streamid, static_cast<uint8_t*>(buf), len, fin); 
    return static_cast<int>(rn);
}

bool net::QuicConnection::fillQuic()
{
    while (1) {
        InetAddress peeraddr;
        ssize_t read = udpstream_->asyncRecvfrom(quicReadBuffer, sizeof(quicReadBuffer), &peeraddr);
        if (read < 0) {
            return false;
        }

        quiche_recv_info recv_info = {
            peeraddr.getSockAddr(),
            peeraddr.getSockLen(),
            localaddr_.getSockAddr(),
            localaddr_.getSockLen(),
        };
        ssize_t done = quiche_conn_recv(conn_, quicReadBuffer, read, &recv_info);
        if (done < 0) {
            continue;
        }
        return true;
    }
}

bool net::QuicConnection::isClosed()
{
   if (quiche_conn_is_closed(conn_)) {
       LOG_INFO << "quic conn closed";
       return true;
   }
   return false;
}