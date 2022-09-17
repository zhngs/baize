#ifndef BAIZE_TIME_WHEEL_H_
#define BAIZE_TIME_WHEEL_H_

#include <memory>

#include "time/timer.h"
#include "util/types.h"

namespace baize
{

namespace time
{

class TimeWheel  // noncopyable
{
public:  // types
    /**
     * time_wheel_[0] < 256ms
     * time_wheel_[1] < 65s + 536ms
     * time_wheel_[2] < 4h + 39min + 37s + 216ms
     * time_wheel_[3] < 49day + 17h + 2min + 47s + 296ms
     */
    static const int kWheelNum = 4;
    static const int kWheelSize = 1 << 8;

    using Uptr = std::unique_ptr<TimeWheel>;

public:  // factory
    static Uptr New();

public:  // special function
    TimeWheel();
    ~TimeWheel();
    TimeWheel(const TimeWheel&) = delete;
    TimeWheel& operator=(const TimeWheel&) = delete;

public:  // normal function
    void AddTimer(Timer* timer);
    void DelTimer(Timer* timer);
    void TurnWheel();

private:  // private function
    Timer* FindHead(Timer* timer);
    void MoveTimerList(Timer* head);

private:
    int64_t last_ms_;
    Timer time_wheel_[kWheelNum][kWheelSize];
};

using TimeWheelUptr = TimeWheel::Uptr;

}  // namespace time

}  // namespace baize

#endif  // BAIZE_TIME_WHEEL_H_