#include "log/logger.h"
#include "net/tcp_listener.h"
#include "runtime/event_loop.h"
#include "time/timer.h"

using namespace baize;
using namespace baize::runtime;
using namespace baize::net;
using namespace baize::time;

void routine_test_timeout()
{
    TcpListener listener(6060);
    listener.Start();

    bool timeout = false;
    TcpStreamSptr stream = listener.AsyncAccept(3000, timeout);
    if (timeout) {
        LOG_INFO << "routine timeout";
        assert(!stream);
    }
}

int main()
{
    EventLoop loop;
    loop.Do(routine_test_timeout);
    loop.Start();
}