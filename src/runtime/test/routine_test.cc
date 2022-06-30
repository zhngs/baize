#include "log/Logger.h"
#include "runtime/Routine.h"

using namespace baize;
using namespace baize::runtime;

void test()
{
    LOG_DEBUG << "test";
}

int main()
{
    int a;
    Routine routine1([&a]{
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

    Routine routine2(test);
    routine2.call();
}