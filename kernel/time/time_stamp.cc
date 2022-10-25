#include "time/time_stamp.h"

#include <stddef.h>
#include <sys/time.h>

namespace baize
{

namespace time
{

Timestamp Timestamp::Now()
{
    struct timeval tv;
    MemoryZero(&tv, sizeof(tv));
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kUsPerSec + tv.tv_usec);
}

string Timestamp::date() const
{
    char buf[64] = "";
    time_t seconds = static_cast<time_t>(us_ / kUsPerSec);
    int us = static_cast<int>(us_ % kUsPerSec);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    snprintf(buf,
             sizeof(buf),
             "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec,
             us);
    return buf;
}

}  // namespace time

}  // namespace baize