#include "runtime/Routine.h"

#include "log/Logger.h"

#include <boost/context/all.hpp>

using namespace baize;

thread_local boost::context::continuation mainRoutine;
thread_local uint64_t g_currentRoutineId = runtime::Routine::kmainRoutineId;
thread_local uint64_t g_routineId = runtime::Routine::kmainRoutineId + 1;

class runtime::Routine::Impl
{
public:
    Impl(RoutineCallBack func)
      : routineid_(g_routineId++),
        isfinished_(false),
        cb_(func)
    {
        LOG_TRACE << "routine " << routineid_ << " creat";
        routine_ = boost::context::callcc([this](boost::context::continuation&& routine){
            mainRoutine = std::move(routine);
            g_currentRoutineId = routineid_;
            hangup();
            cb_();
            isfinished_ = true;
            g_currentRoutineId = kmainRoutineId;
            LOG_TRACE << "routine " << routineid_ << " finish";
            return std::move(mainRoutine);
        });
    }

    void call()
    {
        g_currentRoutineId = routineid_;
        if (isfinished_) {
            LOG_FATAL << "Routine has finished";    
        }
        routine_ = routine_.resume();
    }

    int64_t getRoutineId() { return routineid_; }
public:
    uint64_t routineid_;
    bool isfinished_;
    RoutineCallBack cb_;
    boost::context::continuation routine_;
};


runtime::Routine::Routine(RoutineCallBack func)
    : impl_(std::make_unique<Impl>(func))
{
}

runtime::Routine::~Routine()
{
}

bool runtime::Routine::isInMainRoutine()
{
    return g_currentRoutineId == kmainRoutineId;
}

uint64_t runtime::Routine::getCurrentRoutineId()
{
    return g_currentRoutineId;
}

uint64_t runtime::Routine::getRoutineId()
{
    return impl_->getRoutineId();
}

void runtime::Routine::call()
{
    if (!isInMainRoutine()) {
        LOG_FATAL << "Routine::call must be called by main routine";
    }
    impl_->call();
}

void runtime::Routine::hangup()
{
    if (isInMainRoutine()) {
        LOG_FATAL << "Routine::hangup can't be called by main routine";
    }
    g_currentRoutineId = kmainRoutineId;
    mainRoutine = mainRoutine.resume();
}

