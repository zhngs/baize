#ifndef BAIZE_TIMETYPE_H
#define BAIZE_TIMETYPE_H

#include <time/Timestamp.h>

#include <functional>

namespace baize
{

namespace time
{

typedef std::function<void()> TimerCallback;
typedef std::pair<Timestamp, uint64_t> TimerId;

}  // namespace time

}  // namespace baize

#endif  // BAIZE_TIMETYPE_H