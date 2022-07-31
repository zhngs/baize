#ifndef BAIZE_ROUTINE_H_
#define BAIZE_ROUTINE_H_

#include <functional>
#include <memory>

namespace baize
{

namespace runtime
{

using RoutineId = uint64_t;
using RoutineCallBack = std::function<void()>;

uint64_t current_routineid();
bool is_main_routine();

// go back to main routine
void Return();

class Routine  // noncopyable
{
public:
    static const uint64_t kMainRoutineId = 0;

    // routine cannot be nested
    Routine(RoutineCallBack func);
    ~Routine();

    Routine(const Routine&) = delete;
    Routine& operator=(const Routine&) = delete;

    void Call();

    // getter
    uint64_t routineid();
    bool is_ticks_end() { return ticks_ <= 0; }
    bool is_routine_end();

    // setter
    void set_ticks(int ticks) { ticks_ = ticks; }
    void set_ticks_down() { ticks_--; };

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    int ticks_;
};

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_ROUTINE_H