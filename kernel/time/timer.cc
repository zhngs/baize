#include "time/timer.h"

#include "runtime/event_loop.h"

namespace baize
{

namespace time
{

Timer::~Timer() { Stop(); }

void Timer::Start(int64_t ms)
{
    expiration_ = Timestamp::Now().AddMs(ms);
    runtime::current_loop()->AddTimer(this);
}

void Timer::Stop() { runtime::current_loop()->DelTimer(this); }

bool Timer::Run()
{
    int next_ms = cb_();
    if (next_ms <= 0) {
        return false;
    } else {
        expiration_.AddMs(next_ms);
        return true;
    }
}

}  // namespace time

}  // namespace baize