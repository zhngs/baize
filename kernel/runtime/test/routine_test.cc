#include "runtime/routine.h"

#include "log/logger.h"
#include "runtime/event_loop.h"

using namespace baize;
using namespace baize::runtime;

int main()
{
    EventLoop loop;

    int a;
    Routine routine1([&a] {
        a = 0;
        int b = 1;
        for (;;) {
            current_routine()->Return();
            int next = a + b;
            a = b;
            b = next;
        }
    });
    for (int i = 0; i < 10; i++) {
        routine1.Call();
        LOG_INFO << "a = " << a;
    }
}