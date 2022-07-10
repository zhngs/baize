#include "QuicConnection.h"
#include "log/Logger.h"
#include "runtime/EventLoop.h"

using namespace baize;
using namespace baize::net;
using namespace baize::runtime;
using namespace baize::log;

void discard_quic_client()
{
    LOG_INFO << "diacrad quic client start";
    QuicConnSptr conn = QuicConnection::connect("127.0.0.1", 6060);
    if (!conn) {
        LOG_FATAL << "quic conn get failed";
    }

    const static uint8_t r[] = "GET /index.html\r\n";
    int wn = conn->quicStreamWrite(4, r, sizeof(r), true);
    if (wn < 0) {
        LOG_INFO << "quicStreamWrite failed";
    }

    char buf[1024];
    bool fin = false;
    while (1) {
        bool ret = conn->fillQuic();
        if (!ret) {
            LOG_ERROR << "fillQuic failed";
            break;
        }
        int rn = conn->quicStreamRead(4, buf, sizeof(buf), &fin);
        if (rn < 0) {
            LOG_ERROR << "quicStreamRead error: " << rn;
            if (conn->isClosed()) {
                break;
            }
            continue;
        }
        LOG_INFO << "quicStreamRead: " << string(buf, rn);
    }
}

int main()
{
    Logger::setLogLevel(Logger::INFO);
    EventLoop loop;
    loop.addAndExecRoutine(discard_quic_client);
    loop.start();
}