#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::time;
using namespace baize::runtime;

int test2()
{
    LOG_INFO << "test2";
    return kTimer1S;
}

int test3()
{
    static int times = 0;
    LOG_INFO << "test3";
    times++;
    if (times == 10) {
        return kTimerStop;
    } else {
        return kTimer1MS * 200;
    }
}

int test()
{
    LOG_INFO << "test";
    return kTimerStop;
}

void timer()
{
    LOG_INFO << "timer start";
    Timer timer1(test);
    Timer timer2(test2);
    Timer timer3(test3);

    timer1.Start(kTimer1S);
    timer2.Start(kTimer1S * 2);
    timer3.Start(kTimer1S * 3);
    current_routine()->Return();
}

int main()
{
    log::Logger::set_loglevel(log::Logger::INFO);
    EventLoop loop(10);
    loop.Do(timer);
    loop.Start();
}