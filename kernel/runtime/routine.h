#ifndef BAIZE_ROUTINE_H_
#define BAIZE_ROUTINE_H_

#include <functional>
#include <memory>

#include "time/timer.h"

namespace baize
{

namespace runtime
{

/**
 * global types
 */
using RoutineId = uint64_t;
using RoutineCallBack = std::function<void()>;

class Routine  // noncopyable
{
public:  // types and constant
    static const uint64_t kMainRoutineId = 0;
    static const int kRoutineTicks = 10;
    static const int kStackSize = 128 * 1024;

public:  // special function
    // routine cannot be nested
    explicit Routine(RoutineCallBack func, int stacksize = kStackSize);
    ~Routine();
    Routine(const Routine&) = delete;
    Routine& operator=(const Routine&) = delete;

public:  // normal fucntion
    void Call();
    void Return();
    void Return(int ms, bool& timeout);
    bool Tick();

    // getter
    uint64_t routineid() { return routineid_; };

    // setter
    void set_ticks(int ticks) { ticks_max_ = ticks; }

private:  // private normal function
    int OnTimer();

private:
    class RoutineImpl;
    std::unique_ptr<RoutineImpl> routine_impl_;

    uint64_t routineid_;

    int ticks_max_ = kRoutineTicks;
    int ticks_now_ = 0;

    time::Timer timer_;
    bool timeout_ = false;
};

/**
 * global function
 */
Routine* current_routine();
uint64_t current_routineid();
bool is_main_routine();

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_ROUTINE_H