#include "runtime/EventLoop.h"

#include "log/Logger.h"
#include "net/TcpListener.h"
#include "net/TcpStream.h"
#include "net/Socket.h"
#include "time/Timestamp.h"

using namespace baize;
using namespace baize::net;
using namespace baize::runtime;

const int g_tosend = 1024 * 1024 * 100; 
int64_t g_sendbytes = 0;
int64_t g_readbytes = 0;

void echo_connection(TcpStreamSptr conn)
{
    char buf[1024];
    time::Timestamp begin =  time::Timestamp::now();
    while (1) {
        int rn = conn->asyncReadOrDie(buf, sizeof(buf));
        if (rn == 0) break;
        g_readbytes += rn;
        // LOG_INFO << "read " << g_readbytes << " bytes";
        // int wn = conn->asyncWriteOrDie(buf, rn);
        // LOG_INFO << "write " << wn << " bytes, conten=" << string(buf, wn);
        // assert(rn == wn);
    }
    time::Timestamp end =  time::Timestamp::now();
    int64_t diff = end.getUs() - begin.getUs();
    // double res = static_cast<double>(g_sendbytes) / static_cast<double>(diff) / 1000000;
    // LOG_INFO << "res is " << res << " MiB/s";
    LOG_INFO << "passed " << diff / 1000000 << " s";
}

void echo_server()
{
    TcpListener listener(6070);
    listener.start();

    while (1) {
        TcpStreamSptr stream = listener.asyncAccept();
        EventLoop::getCurrentLoop()->addRoutine([stream]{ echo_connection(stream); });
        LOG_INFO << "accept connection " << stream->getPeerIpPort();
    }
}

void echo_client()
{
    char buf[1024];
    string message(1024, 'z');
    TcpStreamSptr stream = TcpStream::asyncConnect("127.0.0.1", 6070);
    if (!stream) return;
    while (1) {
        int wn = stream->asyncWriteOrDie(message.c_str(), message.size());
        g_sendbytes += wn;
        if (g_sendbytes >= g_tosend) {
            break;
        }
    }

    stream->shutdownWrite();
    while(stream->asyncReadOrDie(buf, sizeof(buf)) != 0) {
    }
}

int main(int argc, char* argv[])
{
    log::Logger::setLogLevel(log::Logger::INFO);
    EventLoop loop;
    if (argc != 2) {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
        return 0;
    }
    if (strcmp(argv[1], "-s") == 0) {
        loop.addAndExecRoutine(echo_server);
    } else if (strcmp(argv[1], "-c") == 0) {
        loop.addAndExecRoutine(echo_client);
    } else {
        LOG_INFO << "usage: " << argv[0] << " [-s|-c]";
    }
    loop.start();
}