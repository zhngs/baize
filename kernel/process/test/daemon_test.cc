#include "process/daemon.h"

#include <unistd.h>

using namespace baize;
using namespace baize::process;

int main()
{
    BecomeDaemon();
    while (1) {
        sleep(1);
    }
}