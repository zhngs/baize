#include "thread/wait_group.h"

#include "log/logger.h"
#include "thread/thread.h"

using namespace baize;
using namespace baize::thread;

int main()
{
    WaitGroup wg(3);
    Thread thread1("wait_group_thread", [&] {
        int i = 0;
        LOG_INFO << i++;
        wg.Done();
        LOG_INFO << i++;
        wg.Done();
        LOG_INFO << i++;
        wg.Done();
    });
    thread1.Start();
    LOG_INFO << "before wait";
    wg.Wait();
    LOG_INFO << "after wait";
    thread1.Join();
}