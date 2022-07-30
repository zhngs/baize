#ifndef BAIZE_TIMER_H_
#define BAIZE_TIMER_H_

#include "time/time_stamp.h"

namespace baize
{

namespace time
{

using TimerCallback = std::function<void()>;
using TimerId = uint64_t;

class Timer  // noncopyable
{
public:
    Timer(TimerCallback cb, Timestamp when, double interval);
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void Run() { cb_(); }
    void Restart(Timestamp now);

    // getter
    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    TimerId timerid() const { return id_; }

private:
    TimerCallback cb_;
    Timestamp expiration_;
    double interval_;
    bool repeat_;
    uint64_t id_;
};

}  // namespace time

}  // namespace baize

#endif  // BAIZE_TIMER_H
