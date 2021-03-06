#include "runtime/Routine.h"

#include "log/Logger.h"

using namespace baize;
using namespace baize::runtime;

int main()
{
    int a;
    Routine routine1([&a] {
        a = 0;
        int b = 1;
        for (;;) {
            Routine::hangup();
            int next = a + b;
            a = b;
            b = next;
        }
    });
    for (int i = 0; i < 10; i++) {
        routine1.call();
        LOG_DEBUG << "a = " << a;
    }
}