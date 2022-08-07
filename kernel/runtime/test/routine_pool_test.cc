#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::runtime;

long nest_times = 0;
long k = 2000;

void test()
{
    nest_times++;
    LOG_INFO << "test nest" << nest_times;
    if (nest_times == k) Return();

    test();
}

int main()
{
    int routines = 1024 * 4;
    EventLoop loop(routines);
    for (int i = 0; i < routines; i++) {
        nest_times = 0;
        loop.Do(test);
    }
    loop.Start();
}