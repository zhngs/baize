#include "process/signal.h"

#include <signal.h>

#include "util/types.h"

namespace baize
{

namespace process
{

void HandleSignal(int signum)
{
    switch (signum) {
        case SIGUSR1:
        case SIGUSR2:
            break;

        default:
            break;
    }
}

void TakeOverSignal()
{
    struct sigaction sa_user;
    MemoryZero(&sa_user, sizeof(sa_user));
    sa_user.sa_handler = HandleSignal;

    struct sigaction sa_ignore;
    MemoryZero(&sa_ignore, sizeof(sa_ignore));
    sa_ignore.sa_handler = SIG_IGN;

    struct sigaction sa_default;
    MemoryZero(&sa_default, sizeof(sa_default));
    sa_default.sa_handler = SIG_DFL;

    sigaction(SIGUSR1, &sa_user, nullptr);
    sigaction(SIGUSR2, &sa_user, nullptr);
    sigaction(SIGPIPE, &sa_ignore, nullptr);
}

}  // namespace process

}  // namespace baize
