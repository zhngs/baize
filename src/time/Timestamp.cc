#include "time/Timestamp.h"

#include <stddef.h>
#include <sys/time.h>

using namespace baize;

time::Timestamp time::Timestamp::now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kusPerSec + tv.tv_usec);
}

string time::Timestamp::toFormatString() {
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(us_ / kusPerSec);
  int us = static_cast<int>(us_ % kusPerSec);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);

  snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, us);
  return buf;
}
