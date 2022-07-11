#include "QuicConnection.h"
#include "QuicListener.h"
#include "log/Logger.h"
#include "runtime/EventLoop.h"
#include "time/Timestamp.h"
#include "thread/Thread.h"
#include <unistd.h>

using namespace baize;
using namespace baize::net;
using namespace baize::runtime;
using namespace baize::log;
using namespace baize::time;
using namespace baize::thread;

int64_t g_sendpackets = 0;
int64_t g_sendbytes = 0;
int64_t g_sendbytes_last = 0;

int64_t g_readbytes = 0;
int64_t g_readbytes_last = 0;
int64_t g_msg = 0;

Timestamp g_last_time;


void quic_client_default_example()
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

void client_print()
{
    Timestamp current_time(Timestamp::now());
    double sec = elapsedInSecond(current_time, g_last_time);
    double send_bytes = static_cast<double>(g_sendbytes - g_sendbytes_last);
    double speed = send_bytes / sec / 1024 / 1024;

    LOG_WARN << "discard client write speed " << speed << " MiB/s";

    g_sendbytes_last = g_sendbytes;
    g_last_time = current_time;
    sleep(1);
}

void discard_quic_client()
{
    QuicConnSptr conn = QuicConnection::connect("127.0.0.1", 6060);
    if (!conn) {
        LOG_FATAL << "quic conn get failed";
    }

    thread::Thread thread_print([]{
        while (1) {
            client_print();
        }
    }, "client_print");
    thread_print.start();

    string message(1024, 'z');
    while (1) {
        int wn = conn->quicStreamWrite(4, message.c_str(), 1024, false);
        assert(wn == 1024);
        g_sendbytes += wn;
        g_sendpackets++;
        LOG_INFO << "quicStreamWrite " << wn << " bytes, " << "packets: " << g_sendpackets;
        conn->fillQuic();
    }
}

void discard_quic_server()
{
    QuicListener listener(6060);
    listener.loopAndAccept();
}

int main(int argc, char* argv[])
{
    Logger::setLogLevel(Logger::WARN);
    EventLoop loop;
    if (argc != 2) {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
        return 0;
    }
    if (strcmp(argv[1], "-s") == 0) {
        loop.addAndExecRoutine(discard_quic_server);
    } else if (strcmp(argv[1], "-c") == 0) {
        loop.addAndExecRoutine(discard_quic_client);
    } else {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
    }
    loop.start();
}