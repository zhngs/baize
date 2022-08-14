#include "log/logger.h"
#include "net/tcp_listener.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::runtime;
using namespace baize::net;

void echo_connection(TcpStreamSptr stream)
{
    net::Buffer* buf = stream->read_buffer();
    while (1) {
        int rn = stream->AsyncRead();
        LOG_INFO << "read " << rn << " bytes from connection "
                 << stream->peer_ip_port();
        if (rn <= 0) break;

        int wn = stream->AsyncWrite(buf->read_index(), buf->readable_bytes());
        LOG_INFO << "write " << wn << " bytes to connection "
                 << stream->peer_ip_port();
        if (wn != buf->readable_bytes()) break;

        // buf->TakeAll();
    }
    LOG_INFO << "connection " << stream->peer_ip_port() << " close";
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
    EventLoop loop(10);
    loop.Do(echo_server);
    loop.Start();
}