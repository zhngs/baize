#ifndef BAIZE_TIMESTAMP_H_
#define BAIZE_TIMESTAMP_H_

#include "util/types.h"

namespace baize
{

namespace time
{

class Timestamp  // copyable
{
public:
    static const int kUsPerSec = 1000000;

    // factory function
    static Timestamp Now();

    Timestamp() = default;
    explicit Timestamp(int64_t us) : us_(us) {}
    explicit Timestamp(timeval tv) : us_(tv.tv_sec * kUsPerSec + tv.tv_usec) {}

    Timestamp AddUs(int64_t us)
    {
        us_ += us;
        return Timestamp(us_);
    }
    Timestamp AddMs(int64_t ms)
    {
        us_ += (1000 * ms);
        return Timestamp(us_);
    }

    // getter
    string date() const;
    int64_t us() const { return us_; }
    int64_t ms() const { return us_ / 1000; }
    bool valid() const { return us_ > 0; }

private:
    int64_t us_ = 0;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.us() < rhs.us();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.us() == rhs.us();
}

inline double ElapsedInSecond(Timestamp high, Timestamp low)
{
    int64_t diff = high.us() - low.us();
    return static_cast<double>(diff) / Timestamp::kUsPerSec;
}

}  // namespace time

}  // namespace baize

#endif  // BAIZE_TIMESTAMP_H