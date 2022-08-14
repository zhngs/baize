#include "log/logger.h"
#include "net/tcp_listener.h"
#include "net/tcp_stream.h"
#include "runtime/event_loop.h"
#include "time/time_stamp.h"

using namespace baize;
using namespace baize::net;
using namespace baize::time;
using namespace baize::runtime;

int64_t g_sendbytes = 0;
int64_t g_sendbytes_last = 0;

int64_t g_readbytes = 0;
int64_t g_readbytes_last = 0;
int64_t g_msg = 0;

Timestamp g_last_time;

void server_print()
{
    Timestamp current_time(Timestamp::Now());
    double sec = ElapsedInSecond(current_time, g_last_time);
    double read_bytes = static_cast<double>(g_readbytes - g_readbytes_last);
    double speed = read_bytes / sec / 1024 / 1024;
    double bytes_msg = read_bytes / static_cast<double>(g_msg);

    LOG_INFO << "discard server read speed " << speed << " MiB/s, " << g_msg
             << " Msg/s, " << bytes_msg << " bytes/msg";

    g_readbytes_last = g_readbytes;
    g_last_time = current_time;
    g_msg = 0;
}

void discard_connection(TcpStreamSptr conn)
{
    char buf[65536];
    g_last_time = Timestamp::Now();
    TimerId id = current_loop()->RunEvery(1, server_print);
    while (1) {
        int rn = conn->AsyncRead(buf, sizeof(buf));
        if (rn <= 0) break;
        g_msg++;
        g_readbytes += rn;
    }
    LOG_INFO << "discard_connection finish";
    current_loop()->CancelTimer(id);
}

void discard_server()
{
    TcpListener listener(6070);
    listener.Start();

    while (1) {
        TcpStreamSptr stream = listener.AsyncAccept();
        stream->set_tcp_nodelay();
        current_loop()->Do([stream] { discard_connection(stream); });
        LOG_INFO << "accept connection " << stream->peer_ip_port();
    }
}

void client_print()
{
    Timestamp current_time(Timestamp::Now());
    double sec = ElapsedInSecond(current_time, g_last_time);
    double send_bytes = static_cast<double>(g_sendbytes - g_sendbytes_last);
    double speed = send_bytes / sec / 1024 / 1024;

    LOG_INFO << "discard client write speed " << speed << " MiB/s";

    g_sendbytes_last = g_sendbytes;
    g_last_time = current_time;
}

void discard_client()
{
    string message(1024, 'z');
    TcpStreamSptr stream = TcpStream::AsyncConnect("127.0.0.1", 6070);
    if (!stream) return;
    stream->set_tcp_nodelay();

    current_loop()->RunEvery(1, client_print);
    g_last_time = Timestamp::Now();
    while (1) {
        int wn = stream->AsyncWrite(message.c_str(), message.size());
        if (wn != static_cast<int>(message.size())) break;
        g_sendbytes += wn;
    }
    LOG_INFO << "discard_client finish";
}

int main(int argc, char* argv[])
{
    log::Logger::set_loglevel(log::Logger::INFO);
    EventLoop loop(10);
    if (argc != 2) {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
        return 0;
    }
    if (strcmp(argv[1], "-s") == 0) {
        loop.Do(discard_server);
    } else if (strcmp(argv[1], "-c") == 0) {
        loop.Do(discard_client);
    } else {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
    }
    loop.Start();
}