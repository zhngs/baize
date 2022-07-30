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

    Timestamp() : us_(0) {}
    explicit Timestamp(int64_t us) : us_(us) {}

    // getter
    string date() const;
    int64_t us() const { return us_; }
    int64_t ms() const { return us_ / 1000; }
    bool valid() const { return us_ > 0; }

private:
    int64_t us_;
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

inline Timestamp AddTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kUsPerSec);
    return Timestamp(timestamp.us() + delta);
}

}  // namespace time

}  // namespace baize

#endif  // BAIZE_TIMESTAMP_H