#ifndef BAIZE_CONDITION_H_
#define BAIZE_CONDITION_H_

#include "thread/mutex.h"

namespace baize
{

namespace thread
{

class Condition  // noncopyable
{
public:
    Condition(MutexLock& mutex) : mutex_(mutex)
    {
        assert(pthread_cond_init(&cond_, nullptr) == 0);
    }
    ~Condition() { assert(pthread_cond_destroy(&cond_) == 0); }
    Condition(const Condition&) = delete;
    Condition& operator=(const Condition&) = delete;

    void Wait() { assert(pthread_cond_wait(&cond_, mutex_.mutex()) == 0); }
    bool WaitForSeconds(double seconds);
    void Notify() { assert(pthread_cond_signal(&cond_) == 0); }
    void NotifyAll() { assert(pthread_cond_broadcast(&cond_) == 0); }

private:
    MutexLock& mutex_;
    pthread_cond_t cond_;
};

}  // namespace thread

}  // namespace baize

#endif  // BAIZE_CONDITION_H