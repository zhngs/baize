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
public:  // special fucntion
    RoutinePool(int poolsize);
    ~RoutinePool();
    RoutinePool(const RoutinePool&) = delete;
    RoutinePool& operator=(const RoutinePool&) = delete;

public:  // normal function
    void Start(RoutineCallBack func);

private:  // private normal function
    void RoutineProcess();

private:
    std::map<RoutineId, std::unique_ptr<Routine>> free_list_;
    std::map<RoutineId, std::unique_ptr<Routine>> active_list_;
    std::list<RoutineCallBack> tasks_;
};

}  // namespace runtime

}  // namespace baize

#endif  // BAIZE_ROUTINE_POOL_H_