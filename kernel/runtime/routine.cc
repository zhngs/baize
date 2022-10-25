#include "runtime/routine.h"

#include <functional>

#include "boost/context/all.hpp"
#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace runtime
{

/**
 * global variable
 */
thread_local Routine* tg_current_routine = nullptr;

thread_local uint64_t tg_routineid_counter = Routine::kMainRoutineId + 1;

class Routine::RoutineImpl
{
public:
    RoutineImpl(RoutineCallBack func, int stacksize)
      : finished_(false), cb_(std::move(func))
    {
        // boost-context的堆栈是动态增长的，并不会一开始就分配完
        boost::context::fixedsize_stack salloc(stacksize);
        // boost::context::protected_fixedsize_stack salloc(stacksize);
        // boost::context::pooled_fixedsize_stack salloc(stacksize);

        routine_ = boost::context::callcc(
            std::allocator_arg,
            salloc,
            [this](boost::context::continuation&& routine) {
                main_routine_ = routine.resume();  // return to main routine

                cb_();
                // 提前释放function内存储的资源
                cb_ = RoutineCallBack();

                finished_ = true;
                tg_current_routine = nullptr;
                return std::move(main_routine_);
            });
    }

    void Call()
    {
        if (finished_) {
            LOG_ERROR << "Routine has finished";
            return;
        }
        routine_ = routine_.resume();
    }

    void Return()
    {
        if (finished_) {
            LOG_ERROR << "Routine has finished";
            return;
        }
        main_routine_ = main_routine_.resume();
    }

public:
    bool finished_;
    RoutineCallBack cb_;
    boost::context::continuation routine_;
    boost::context::continuation main_routine_;
};

Routine::Routine(RoutineCallBack func, string name, int stacksize)
  : routine_impl_(std::make_unique<RoutineImpl>(func, stacksize)),
    routineid_(tg_routineid_counter++),
    routine_name_(name),
    timer_([this] { return OnTimer(); })
{
    LOG_TRACE << "create routine" << routineid_;
}

Routine::~Routine() { LOG_TRACE << "routine" << routineid_ << " finish"; }

bool Routine::Tick()
{
    ticks_now_++;
    if (ticks_now_ >= ticks_max_) {
        ticks_now_ = 0;
        return true;
    } else {
        return false;
    }
}

void Routine::Call()
{
    if (!is_main_routine()) {
        LOG_FATAL << "Routine::call must be called by main routine";
    }
    LOG_TRACE << "<<<<<<<<<< call routine" << routineid_;

    /* last routine */
    /**************************************************************/
    /* next routine */

    tg_current_routine = this;
    routine_impl_->Call();
}

void Routine::Return()
{
    if (is_main_routine()) {
        LOG_FATAL << "Return can't be called by main routine";
    }
    LOG_TRACE << ">>>>>>>>>> exit routine" << routineid_;

    /* last routine */
    /**************************************************************/
    /* next routine */

    tg_current_routine = nullptr;
    routine_impl_->Return();
}

void Routine::Return(int ms, bool& timeout)
{
    timeout_ = false;
    timer_.Start(ms);
    Return();
    timer_.Stop();
    timeout = timeout_;
}

int Routine::OnTimer()
{
    timeout_ = true;
    Call();
    return time::kTimerStop;
}

/**
 * free function
 */
uint64_t current_routineid()
{
    return current_routine() ? current_routine()->routineid()
                             : Routine::kMainRoutineId;
}

string current_routine_name() { return current_routine()->name(); }

Routine* current_routine() { return tg_current_routine; }

bool is_main_routine() { return current_routine() == nullptr; }

}  // namespace runtime

}  // namespace baize