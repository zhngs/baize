#include "time/time_wheel.h"

#include "log/logger.h"

namespace baize
{

namespace time
{

TimeWheel::Uptr TimeWheel::New() { return std::make_unique<TimeWheel>(); }

TimeWheel::TimeWheel()
{
    MemZero(time_wheel_, sizeof(time_wheel_));
    last_ms_ = Timestamp::Now().ms();
    assert(last_ms_ > 0);
}

TimeWheel::~TimeWheel() {}

void TimeWheel::AddTimer(Timer* timer)
{
    DelTimer(timer);

    Timer* head = FindHead(timer);
    if (!head) {
        LOG_ERROR << "add timer failed";
        return;
    }

    if (head->next_) {
        timer->next_ = head->next_;
        head->next_->prev_ = timer;
        head->next_ = timer;
        timer->prev_ = head;
    } else {
        timer->next_ = nullptr;
        timer->prev_ = head;
        head->next_ = timer;
    }
}

void TimeWheel::DelTimer(Timer* timer)
{
    if (timer->prev_ == nullptr) {
        timer->next_ = nullptr;
        return;
    }

    if (timer->next_ != nullptr) {
        timer->next_->prev_ = timer->prev_;
        timer->prev_->next_ = timer->next_;
    } else {
        timer->prev_->next_ = nullptr;
    }

    timer->prev_ = nullptr;
    timer->next_ = nullptr;
}

static inline int64_t MaxTime(int wheel_index)
{
    return static_cast<int64_t>(1) << ((wheel_index + 1) * 8);
}

static inline uint8_t CalcTime(int64_t ms, int wheel_index)
{
    return static_cast<uint8_t>(ms >> (wheel_index * 8));
}

void TimeWheel::TurnWheel()
{
    int64_t now = Timestamp::Now().ms();
    assert(now > 0);
    if (now < last_ms_) return;

    int64_t update_times = now - last_ms_;
    for (int i = 0; i < update_times; i++) {
        last_ms_++;
        for (int j = 0; j < kWheelNum; j++) {
            uint8_t index = CalcTime(last_ms_, j);

            LOG_DEBUG << log::TempFmt(
                "CalcTime %#lx time_wheel_[%d][%d]", last_ms_, j, index);

            Timer* head = &time_wheel_[j][index];
            while (head->next_ != nullptr) {
                Timer* timer = head->next_;
                if (j == 0) {
                    timer->Run();
                    if (timer->repeat()) {
                        timer->Restart();
                    } else {
                        timer->Stop();
                    }
                } else {
                    timer->Stop();
                    timer->Start();
                }
            }

            if (index == 0) {
                continue;
            } else {
                break;
            }
        }
    }
}

Timer* TimeWheel::FindHead(Timer* timer)
{
    int64_t diff = timer->expiration().ms() - last_ms_;
    if (diff <= 0) {
        diff = 1;
    }

    for (int i = 0; i < kWheelNum; i++) {
        if (diff < MaxTime(i)) {
            uint8_t index = CalcTime(timer->expiration().ms(), i);
            LOG_DEBUG << "time wheel FindHead &time_wheel_[" << i << "]["
                      << index << "]";
            return &time_wheel_[i][index];
        }
    }

    return nullptr;
}

}  // namespace time

}  // namespace baize
