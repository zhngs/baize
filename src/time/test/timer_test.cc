#include "log/logger.h"
#include "runtime/EventLoop.h"
#include "time/timer_queue.h"

using namespace baize;
using namespace baize::time;
using namespace baize::runtime;

void test() { LOG_INFO << "test"; }

void test2() { LOG_INFO << "test2"; }

void timer()
{
    EventLoop* loop = getCurrentLoop();
    LOG_INFO << "timer start";
    TimerId id = loop->runAfter(2, test2);
    loop->cancelTimer(id);
    loop->runEvery(1, test);
}

int main()
{
    // log::Logger::set_loglevel(log::Logger::INFO);
    EventLoop loop;
    loop.addAndExecRoutine(timer);
    loop.start();
}