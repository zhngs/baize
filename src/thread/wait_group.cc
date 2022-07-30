#include "thread/wait_group.h"

namespace baize
{

namespace thread
{

thread::WaitGroup::WaitGroup(int count) : mutex_(), cond_(mutex_), count_(count)
{
}

void WaitGroup::Wait()
{
    MutexLockGuard lock(mutex_);
    while (count_ > 0) {
        cond_.Wait();
    }
}

void thread::WaitGroup::Done()
{
    MutexLockGuard lock(mutex_);
    count_--;
    if (count_ == 0) {
        cond_.NotifyAll();
    }
}

int thread::WaitGroup::count()
{
    MutexLockGuard lock(mutex_);
    return count_;
}

}  // namespace thread

}  // namespace baize