#include "runtime/EventLoop.h"

#include "log/Logger.h"
#include "net/TcpListener.h"
#include "net/TcpStream.h"

using namespace baize;
using namespace baize::net;
using namespace baize::runtime;

void echo_connection(TcpStreamSptr conn)
{
    char buf[1024];
    while (1) {
        int rn = conn->asyncReadOrDie(buf, sizeof(buf));
        if (rn == 0) break;
        LOG_INFO << "read " << rn << " bytes, conten=" << string(buf, rn);
        int wn = conn->asyncWriteOrDie(buf, rn);
        LOG_INFO << "write " << wn << " bytes, conten=" << string(buf, wn);
        assert(rn == wn);
    }
}

void echo_server()
{
    TcpListener listener(6060);
    listener.start();

    while (1) {
        TcpStreamSptr stream = listener.asyncAccept();
        EventLoop::getCurrentLoop()->addRoutine([stream]{ echo_connection(stream); });
        LOG_INFO << "accept connection " << stream->getPeerIpPort();
    }
}

int main()
{
    EventLoop loop;
    loop.addAndExecRoutine(echo_server);
    loop.start();
}