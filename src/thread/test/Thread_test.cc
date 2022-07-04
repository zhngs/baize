#include "runtime/EventLoop.h"
#include "thread/Thread.h"
#include "log/Logger.h"

using namespace baize;
using namespace baize::runtime;
using namespace baize::thread;

void test()
{
}

int main()
{
    EventLoop loop;
    loop.addAndExecRoutine(test);

    Thread thread1([]{
        EventLoop loop2;
        loop2.addAndExecRoutine(test);
        LOG_INFO << "current thread id=" << getCurrentTid() << ", current thread name=" << getCurrentThreadName();
        loop2.start();
    }, "Thread_test2");
    thread1.start();

    LOG_INFO << "current thread id=" << getCurrentTid() << ", current thread name=" << getCurrentThreadName();
    loop.start();
    thread1.join();
}