#ifndef BAIZE_ROUTINE_POOL_H_
#define BAIZE_ROUTINE_POOL_H_

#include <list>
#include <map>
#include <memory>

#include "runtime/routine.h"

namespace baize
{

namespace runtime
{

class RoutinePool  // noncopyable
{
public:
    RoutinePool(int poolsize);
    ~RoutinePool();
    RoutinePool(const RoutinePool&) = delete;
    RoutinePool& operator=(const RoutinePool&) = delete;

    void Start(RoutineCallBack func);
    void Call(RoutineId routineid);

    // getter
    Routine& routine(RoutineId routineid);

private:
    std::map<RoutineId, std::unique_ptr<Routine>> free_list_;
    std::map<RoutineId, std::unique_ptr<Routine>> active_list_;
    std::list<RoutineCallBack> tasks_;
};

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_ROUTINE_POOL_H_