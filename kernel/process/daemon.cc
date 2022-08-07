#include "process/daemon.h"

#include <unistd.h>

#include "log/logger.h"
#include "thread/thread.h"

namespace baize
{

namespace process
{

void BecomeDaemon()
{
    LOG_TRACE << "start to create daemon";
    int err = daemon(0, 1);
    if (err) {
        LOG_SYSFATAL << "create daemon failed";
    }
    thread::CallAfterFork();
    LOG_TRACE << "daemon created";
}

}  // namespace process

}  // namespace baize
