#ifndef BAIZE_TIMESTAMP_H
#define BAIZE_TIMESTAMP_H
// copy from muduo and make some small changes

#include "util/types.h"

namespace baize {

namespace time {

class Timestamp // copyable
{
public:
  static const int kusPerSec = 1000000;

  Timestamp() : us_(0) {}
  Timestamp(int64_t us) : us_(us) {}

  static Timestamp now();
  static Timestamp invalid() { return Timestamp(); }

  string toFormatString();
  int64_t getUs() { return us_; }
  bool valid() { return us_ > 0; }

private:
  int64_t us_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
  return lhs.getUs() < rhs.getUs();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
  return lhs.getUs() == rhs.getUs();
}

inline double elapsedInSecond(Timestamp high, Timestamp low) {
  int64_t diff = high.getUs() - low.getUs();
  return static_cast<double>(diff) / Timestamp::kusPerSec;
}

inline Timestamp addTime(Timestamp timestamp, double seconds) {
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kusPerSec);
  return Timestamp(timestamp.getUs() + delta);
}

} // namespace time

} // namespace baize

#endif // BAIZE_TIMESTAMP_H