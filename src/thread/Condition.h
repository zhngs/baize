#ifndef BAIZE_CONDITION_H
#define BAIZE_CONDITION_H

#include "thread/Mutex.h"

namespace baize
{

namespace thread
{

class Condition //noncopyable
{
public:
    Condition(MutexLock& mutex) : mutex_(mutex) { assert(pthread_cond_init(&cond_, nullptr) == 0); }
    ~Condition() { assert(pthread_cond_destroy(&cond_) == 0); }
    Condition(const Condition&) = delete;
    Condition& operator=(const Condition&) = delete;

    void wait() { assert(pthread_cond_wait(&cond_, mutex_.getMutex()) == 0); }
    bool waitForSeconds(double seconds);
    void notify() { assert(pthread_cond_signal(&cond_) == 0); }
    void notifyAll() { assert(pthread_cond_broadcast(&cond_) == 0); }
private:
    MutexLock& mutex_;
    pthread_cond_t cond_;
};
    
} // namespace thread
    
} // namespace baize


#endif //BAIZE_CONDITION_H