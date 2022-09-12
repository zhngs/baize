#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::time;
using namespace baize::runtime;

void test2() { LOG_INFO << "test2"; }

void test() { LOG_INFO << "test"; }

void timer()
{
    LOG_INFO << "timer start";
    Timer timer1(1000, false, test);
    Timer timer2(1000, true, test2);
    Timer timer3(9000, false, [&] { timer2.Stop(); });

    timer1.Start();
    timer2.Start();
    timer3.Start();
    Return();
}

int main()
{
    log::Logger::set_loglevel(log::Logger::INFO);
    EventLoop loop(10);
    loop.Do(timer);
    loop.Start();
}