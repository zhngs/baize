#include "log/logger.h"
#include "net/tcp_listener.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::runtime;
using namespace baize::net;

void echo_connection(TcpStreamSptr stream)
{
    char buf[1024];
    while (1) {
        bool timeout = false;
        int rn = stream->AsyncRead(buf, sizeof(buf), 5000, timeout);
        if (timeout) {
            LOG_INFO << "connection " << stream->peer_ip_port() << " timeout";
            break;
        }
        if (rn < 0) {
            break;
        }
        if (rn == 0) {
            LOG_INFO << "connection " << stream->peer_ip_port() << " close";
            break;
        }
        int wn = stream->AsyncWrite(buf, rn);
        assert(wn == rn);
    }
}

void echo_server()
{
    TcpListener listener(6060);
    listener.Start();

    while (1) {
        TcpStreamSptr stream = listener.AsyncAccept();
        LOG_INFO << "connection " << stream->peer_ip_port() << " accept";
        current_loop()->Do([stream] { echo_connection(stream); });
    }
}

int main()
{
    log::Logger::set_loglevel(log::Logger::INFO);
    EventLoop loop;
    loop.Do(echo_server);
    loop.Start();
}