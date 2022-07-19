#ifndef BAIZE_WAITGROUP_H
#define BAIZE_WAITGROUP_H

#include "thread/Condition.h"

namespace baize {

namespace thread {

class WaitGroup // noncopyable
{
public:
  explicit WaitGroup(int count);
  WaitGroup(const WaitGroup &) = delete;
  WaitGroup &operator=(const WaitGroup &) = delete;

  void wait();
  void done();
  int getCount();

private:
  MutexLock mutex_;
  Condition cond_;
  int count_;
};

} // namespace thread

} // namespace baize

#endif // BAIZE_WAITGROUP_H