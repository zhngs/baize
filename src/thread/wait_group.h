#ifndef BAIZE_WAITGROUP_H_
#define BAIZE_WAITGROUP_H_

#include "thread/condition.h"

namespace baize
{

namespace thread
{

class WaitGroup  // noncopyable
{
public:
    explicit WaitGroup(int count);
    WaitGroup(const WaitGroup&) = delete;
    WaitGroup& operator=(const WaitGroup&) = delete;

    void Wait();
    void Done();

    // getter
    int count();

private:
    MutexLock mutex_;
    Condition cond_;
    int count_;
};

}  // namespace thread

}  // namespace baize

#endif  // BAIZE_WAITGROUP_H