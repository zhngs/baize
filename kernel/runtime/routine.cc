#include "runtime/routine.h"

#include <functional>

#include "boost/context/all.hpp"
#include "log/logger.h"
#include "runtime/event_loop.h"

namespace baize
{

namespace runtime
{

thread_local boost::context::continuation main_routine;
thread_local uint64_t g_current_routineid = Routine::kMainRoutineId;
thread_local uint64_t g_routineid = Routine::kMainRoutineId + 1;

uint64_t current_routineid() { return g_current_routineid; }
bool is_main_routine()
{
    return g_current_routineid == Routine::kMainRoutineId;
}

class Routine::Impl
{
public:
    Impl(RoutineCallBack func, int stacksize)
      : routineid_(g_routineid++), finished_(false), cb_(std::move(func))
    {
        LOG_TRACE << "create routine" << routineid_;

        // boost-context的堆栈是动态增长的，并不会一开始就分配完
        boost::context::fixedsize_stack salloc(stacksize);
        // boost::context::protected_fixedsize_stack salloc(stacksize);
        // boost::context::pooled_fixedsize_stack salloc(stacksize);

        routine_ = boost::context::callcc(
            std::allocator_arg,
            salloc,
            [this](boost::context::continuation&& routine) {
                main_routine = routine.resume();
                g_current_routineid = routineid_;

                cb_();
                // 提前释放function内存储的资源
                cb_ = RoutineCallBack();

                finished_ = true;
                LOG_TRACE << "routine" << routineid_ << " finish";
                g_current_routineid = kMainRoutineId;
                return std::move(main_routine);
            });
    }

    ~Impl() { LOG_TRACE << "destory routine" << routineid_; }

    void Call()
    {
        LOG_TRACE << "call routine" << routineid_;
        g_current_routineid = routineid_;
        if (finished_) {
            LOG_ERROR << "Routine has finished";
            return;
        }
        routine_ = routine_.resume();
    }

    int64_t routineid() { return routineid_; }

public:
    uint64_t routineid_;
    bool finished_;
    RoutineCallBack cb_;
    boost::context::continuation routine_;
};

Routine::Routine(RoutineCallBack func, int stacksize)
  : impl_(std::make_unique<Impl>(func, stacksize)), ticks_(kRoutineTicks)
{
}

Routine::~Routine() {}

RoutineId Routine::routineid() { return impl_->routineid(); }

void Routine::Call()
{
    if (!is_main_routine()) {
        LOG_FATAL << "Routine::call must be called by main routine";
    }
    impl_->Call();
}

bool Routine::is_routine_end() { return impl_->finished_; }

void Return()
{
    if (is_main_routine()) {
        LOG_FATAL << "Return can't be called by main routine";
    }
    LOG_TRACE << "exit routine" << g_current_routineid;
    g_current_routineid = Routine::kMainRoutineId;
    main_routine = main_routine.resume();
}

}  // namespace runtime

}  // namespace baize