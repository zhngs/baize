#ifndef BAIZE_ROUTINE_H
#define BAIZE_ROUTINE_H

#include "runtime/RuntimeType.h"

#include <boost/context/all.hpp>

namespace baize
{
    
namespace runtime
{

class Routine // noncopyable
{
public:
    static const uint64_t kmainRoutineId = 0;

    //routine cannot be nested
    Routine(RoutineCallBack func);
    ~Routine();

    Routine(const Routine&) = delete;
    Routine& operator=(const Routine&) = delete;
    
    void call();
    static void hangup();
    
    uint64_t getRoutineId();
    static uint64_t getCurrentRoutineId();

    static bool isInMainRoutine();
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace runtime

} // namespace baize


#endif //BAIZE_ROUTINE_H