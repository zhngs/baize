#include "log/logger.h"
#include "net/tcp_listener.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::runtime;
using namespace baize::net;

void echo_connection(TcpStreamSptr stream)
{
    Buffer read_buf;
    while (1) {
        int rn = stream->AsyncRead(read_buf);
        LOG_INFO << "read " << rn << " bytes from connection "
                 << stream->peer_ip_port();
        if (rn <= 0) break;

        int wn = stream->AsyncWrite(read_buf.read_index(),
                                    read_buf.readable_bytes());
        LOG_INFO << "write " << wn << " bytes to connection "
                 << stream->peer_ip_port();
        if (wn != read_buf.readable_bytes()) break;

        read_buf.TakeAll();
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