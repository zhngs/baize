#include "thread/thread.h"

#include "log/logger.h"
#include "runtime/EventLoop.h"

using namespace baize;
using namespace baize::runtime;
using namespace baize::thread;

void test() {}

int main()
{
    EventLoop loop;
    loop.addAndExecRoutine(test);

    Thread thread1("thread_test2", [] {
        EventLoop loop2;
        loop2.addAndExecRoutine(test);
        LOG_INFO << "current thread id=" << tid()
                 << ", current thread name=" << threadname();
        loop2.start();
    });
    thread1.Start();

    LOG_INFO << "current thread id=" << tid()
             << ", current thread name=" << threadname();
    loop.start();
    thread1.Join();
}