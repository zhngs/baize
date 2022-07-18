#include "runtime/Routine.h"

#include "log/Logger.h"
#include "runtime/EventLoop.h"

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
        boost::context::fixedsize_stack salloc(128 * 1024 * 1024);
        routine_ = boost::context::callcc(std::allocator_arg, salloc, [this](boost::context::continuation&& routine){
            mainRoutine = std::move(routine);
            g_currentRoutineId = routineid_;
            LOG_TRACE << "enter routine " << routineid_;
            hangup();

            cb_();

            getCurrentLoop()->runInMainRoutine([=]{
                getCurrentLoop()->removeRoutine(routineid_);
            });
            isfinished_ = true;
            g_currentRoutineId = kmainRoutineId;
            LOG_TRACE << "routine " << routineid_ << " finish and exit";
            return std::move(mainRoutine);
        });
    }

    ~Impl()
    {
        LOG_TRACE << "routine" << routineid_ << " destory";
    }

    void call()
    {
        LOG_TRACE << "call routine " << routineid_;
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
    : impl_(std::make_unique<Impl>(func)),
      timeout_(10)
{
}

runtime::Routine::~Routine()
{
}

bool runtime::Routine::isMainRoutine()
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
    if (!isMainRoutine()) {
        LOG_FATAL << "Routine::call must be called by main routine";
    }
    impl_->call();
}

void runtime::Routine::hangup()
{
    if (isMainRoutine()) {
        LOG_FATAL << "Routine::hangup can't be called by main routine";
    }
    LOG_TRACE << "routine " << g_currentRoutineId << " hangup to main routine";
    g_currentRoutineId = kmainRoutineId;
    mainRoutine = mainRoutine.resume();
}

