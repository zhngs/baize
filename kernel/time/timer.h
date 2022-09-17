#ifndef BAIZE_TIMER_H_
#define BAIZE_TIMER_H_

#include "time/time_stamp.h"

namespace baize
{

namespace time
{

using TimerCallback = std::function<int()>;
const int kTimerStop = 0;
const int kTimer1MS = 1;
const int kTimer1S = 1000;

class Timer  // noncopyable
{
public:  // types
    friend class TimeWheel;

public:
    Timer() = default;
    explicit Timer(TimerCallback cb) : cb_(cb) {}
    ~Timer();
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    bool Run();

    void Start(int64_t ms);
    void Stop();

    // getter
    Timestamp expiration() const { return expiration_; }

private:
    Timer* prev_ = nullptr;
    Timer* next_ = nullptr;

    Timestamp expiration_;
    TimerCallback cb_;
};

}  // namespace time

}  // namespace baize

#endif  // BAIZE_TIMER_H
