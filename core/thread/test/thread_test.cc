#include "thread/thread.h"

#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::runtime;
using namespace baize::thread;

void test() {}

int main()
{
    EventLoop loop;
    loop.Do(test);

    Thread thread1("thread_test2", [] {
        EventLoop loop2;
        loop2.Do(test);
        LOG_INFO << "current thread id=" << tid()
                 << ", current thread name=" << threadname();
        loop2.Start();
    });
    thread1.Start();

    LOG_INFO << "current thread id=" << tid()
             << ", current thread name=" << threadname();
    loop.Start();
    thread1.Join();
}