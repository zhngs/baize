#ifndef BAIZE_MUTEX_H
#define BAIZE_MUTEX_H

#include <assert.h>
#include <pthread.h>

namespace baize
{

namespace thread
{

class MutexLock  // noncopyable
{
public:
    MutexLock() { assert(pthread_mutex_init(&mutex_, nullptr) == 0); }
    ~MutexLock() { assert(pthread_mutex_destroy(&mutex_) == 0); }
    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;

    void lock() { assert(pthread_mutex_lock(&mutex_) == 0); }
    void unlock() { assert(pthread_mutex_unlock(&mutex_) == 0); }

    pthread_mutex_t* getMutex() { return &mutex_; }

private:
    pthread_mutex_t mutex_;
};

class MutexLockGuard  // noncopyable
{
public:
    MutexLockGuard(MutexLock& mutex) : mutex_(mutex) { mutex_.lock(); }
    ~MutexLockGuard() { mutex_.unlock(); }
    MutexLockGuard(const MutexLockGuard&) = delete;
    MutexLockGuard& operator=(const MutexLockGuard&) = delete;

private:
    MutexLock& mutex_;
};

#define MutexLockGuard(x) error "Missing guard object name"

}  // namespace thread

}  // namespace baize

#endif  // BAIZE_MUTEX_H
