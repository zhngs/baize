#include "log/logger.h"
#include "runtime/event_loop.h"
#include "time/timer_queue.h"

using namespace baize;
using namespace baize::time;
using namespace baize::runtime;

void test() { LOG_INFO << "test"; }

void test2() { LOG_INFO << "test2"; }

void timer()
{
    EventLoop* loop = current_loop();
    LOG_INFO << "timer start";
    TimerId id = loop->RunAfter(2, test2);
    loop->CancelTimer(id);
    loop->RunEvery(1, test);
}

int main()
{
    // log::Logger::set_loglevel(log::Logger::INFO);
    EventLoop loop;
    loop.Do(timer);
    loop.Start();
}