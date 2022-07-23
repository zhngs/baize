#include <unistd.h>

#include "log/Logger.h"
#include "net/UdpStream.h"
#include "runtime/EventLoop.h"
#include "thread/Thread.h"
#include "time/Timestamp.h"

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

void server_print()
{
    Timestamp current_time(Timestamp::now());
    double sec = elapsedInSecond(current_time, g_last_time);
    double read_bytes = static_cast<double>(g_readbytes - g_readbytes_last);
    double speed = read_bytes / sec / 1024 / 1024;
    double bytes_msg = read_bytes / static_cast<double>(g_read_msg);

    LOG_INFO << "discard server read speed " << speed << " MiB/s, "
             << g_read_msg << " Msg/s, " << bytes_msg << " bytes/msg";

    g_readbytes_last = g_readbytes;
    g_last_time = current_time;
    g_read_msg = 0;
}

void discard_server()
{
    char buf[4096];
    g_last_time = Timestamp::now();
    getCurrentLoop()->runEvery(1, server_print);
    UdpStreamSptr stream = UdpStream::asServer(6060);
    InetAddress clientaddr;
    while (1) {
        int rn = stream->asyncRecvfrom(buf, sizeof(buf), &clientaddr);

        // LOG_INFO << "discard_server recv " << rn << " bytes from " <<
        // clientaddr.getIpPort();
        g_read_msg++;
        g_readbytes += rn;
    }
    LOG_INFO << "discard_connection finish";
}

void client_print()
{
    Timestamp current_time(Timestamp::now());
    double sec = elapsedInSecond(current_time, g_last_time);
    double send_bytes = static_cast<double>(g_sendbytes - g_sendbytes_last);
    double speed = send_bytes / sec / 1024 / 1024;
    double bytes_msg = send_bytes / static_cast<double>(g_send_msg);

    LOG_INFO << "discard client write speed " << speed << " MiB/s, "
             << g_send_msg << " Msg/s, " << bytes_msg << " bytes/msg";

    g_sendbytes_last = g_sendbytes;
    g_last_time = current_time;
    g_send_msg = 0;
}

void discard_client()
{
    string message(1024, 'z');
    UdpStreamSptr stream = UdpStream::asClient();

    getCurrentLoop()->runEvery(1, client_print);

    g_last_time = Timestamp::now();
    InetAddress serveraddr("127.0.0.1", 6060);
    LOG_INFO << "discard_client start";
    while (1) {
        int wn = stream->asyncSendto(
            message.c_str(), static_cast<int>(message.size()), serveraddr);
        // LOG_INFO << "discard_client sendto " << wn << " bytes to " <<
        // serveraddr.getIpPort();
        g_sendbytes += wn;
        g_send_msg++;
    }
    LOG_INFO << "discard_client finish";
}

int main(int argc, char* argv[])
{
    log::Logger::setLogLevel(log::Logger::DEBUG);
    EventLoop loop;
    if (argc != 2) {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
        return 0;
    }
    if (strcmp(argv[1], "-s") == 0) {
        loop.addAndExecRoutine(discard_server);
    } else if (strcmp(argv[1], "-c") == 0) {
        loop.addAndExecRoutine(discard_client);
    } else {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
    }
    loop.start();
}