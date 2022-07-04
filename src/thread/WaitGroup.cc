#include "thread/WaitGroup.h"

using namespace baize;

thread::WaitGroup::WaitGroup(int count)
  : mutex_(),
    cond_(mutex_),
    count_(count)
{
}

void thread::WaitGroup::wait()
{
    MutexLockGuard lock(mutex_);
    while (count_ > 0) {
        cond_.wait();
    }
}

void thread::WaitGroup::done()
{
    MutexLockGuard lock(mutex_);
    count_--;
    if (count_ == 0) {
        cond_.notifyAll();
    }
}

int thread::WaitGroup::getCount()
{
    MutexLockGuard lock(mutex_);
    return count_;
}
