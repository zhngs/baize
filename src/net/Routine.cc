#include "net/Routine.h"

#include <boost/context/all.hpp>

using namespace baize;

thread_local boost::context::continuation next_routine;

class net::Routine::Impl
{
public:
    Impl(RoutineCb func)
      : cb_(func)
    {
        routine_ = boost::context::callcc([this](boost::context::continuation&& routine){
            next_routine = std::move(routine);
            cb_();
            return std::move(next_routine);
        });
    }

    void call()
    {
        routine_ = routine_.resume();
    }
public:
    RoutineCb cb_;
    boost::context::continuation routine_;
};


net::Routine::Routine(RoutineCb func)
  : impl_(std::make_unique<Impl>(func))
{
}

net::Routine::~Routine()
{
}

void net::Routine::call()
{
    impl_->call();
}

void net::Routine::hangup()
{
    next_routine = next_routine.resume();
}

