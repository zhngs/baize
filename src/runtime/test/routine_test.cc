#include "runtime/routine.h"

#include "log/logger.h"

using namespace baize;
using namespace baize::runtime;

int main()
{
    int a;
    Routine routine1([&a] {
        a = 0;
        int b = 1;
        for (;;) {
            Return();
            int next = a + b;
            a = b;
            b = next;
        }
    });
    for (int i = 0; i < 10; i++) {
        routine1.Call();
        LOG_DEBUG << "a = " << a;
    }
}