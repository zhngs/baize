#include <unistd.h>

#include "log/logger.h"
#include "net/udp_stream.h"
#include "runtime/event_loop.h"
#include "thread/thread.h"
#include "time/time_stamp.h"

using namespace baize;
using namespace baize::net;
using namespace baize::time;
using namespace baize::runtime;

int64_t g_sendbytes = 0;
int64_t g_sendbytes_last = 0;
int64_t g_send_msg = 0;

int64_t g_readbytes = 0;
int64_t g_readbytes_last = 0;
int64_t g_read_msg = 0;

Timestamp g_last_time;

int server_print()
{
    Timestamp current_time(Timestamp::Now());
    double sec = ElapsedInSecond(current_time, g_last_time);
    double read_bytes = static_cast<double>(g_readbytes - g_readbytes_last);
    double speed = read_bytes / sec / 1024 / 1024;
    double bytes_msg = read_bytes / static_cast<double>(g_read_msg);

    LOG_INFO << "discard server read speed " << speed << " MiB/s, "
             << g_read_msg << " Msg/s, " << bytes_msg << " bytes/msg";

    g_readbytes_last = g_readbytes;
    g_last_time = current_time;
    g_read_msg = 0;

    return kTimer1S;
}

void discard_server()
{
    char buf[4096];
    Timer timer(server_print);
    timer.Start(1000);
    g_last_time = Timestamp::Now();

    UdpStreamSptr stream = UdpStream::AsServer(6060);
    InetAddress clientaddr;
    while (1) {
        int rn = stream->AsyncRecvFrom(buf, sizeof(buf), &clientaddr);
        if (rn < 0) break;
        g_read_msg++;
        g_readbytes += rn;
    }
    LOG_INFO << "discard_connection finish";
}

int client_print()
{
    Timestamp current_time(Timestamp::Now());
    double sec = ElapsedInSecond(current_time, g_last_time);
    double send_bytes = static_cast<double>(g_sendbytes - g_sendbytes_last);
    double speed = send_bytes / sec / 1024 / 1024;
    double bytes_msg = send_bytes / static_cast<double>(g_send_msg);

    LOG_INFO << "discard client write speed " << speed << " MiB/s, "
             << g_send_msg << " Msg/s, " << bytes_msg << " bytes/msg";

    g_sendbytes_last = g_sendbytes;
    g_last_time = current_time;
    g_send_msg = 0;

    return kTimer1S;
}

void discard_client()
{
    string message(1024, 'z');
    UdpStreamSptr stream = UdpStream::AsClient();

    Timer timer(client_print);
    timer.Start(1000);
    g_last_time = Timestamp::Now();

    InetAddress serveraddr("127.0.0.1", 6060);
    LOG_INFO << "discard_client start";
    while (1) {
        int wn = stream->AsyncSendto(
            message.c_str(), static_cast<int>(message.size()), serveraddr);
        if (wn != static_cast<int>(message.size())) break;
        g_sendbytes += wn;
        g_send_msg++;
    }
    LOG_INFO << "discard_client finish";
}

int main(int argc, char* argv[])
{
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