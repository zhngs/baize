#include "runtime/routine_pool.h"

#include <assert.h>

#include "log/logger.h"

namespace baize
{

namespace runtime
{

RoutinePool::RoutinePool(int poolsize)
{
    for (int i = 0; i < poolsize; i++) {
        std::unique_ptr<Routine> routine(
            std::make_unique<Routine>([this] { RoutineProcess(); }));

        free_list_.insert({routine->routineid(), std::move(routine)});
    }
}

RoutinePool::~RoutinePool() {}

void RoutinePool::Start(RoutineCallBack func, string routine_name)
{
    tasks_.push_back(std::move(func));
    while (!tasks_.empty() && !free_list_.empty()) {
        auto& routine = free_list_.begin()->second;
        routine->set_name(routine_name);
        routine->Call();
    }
}

void RoutinePool::RoutineProcess()
{
    while (1) {
        RoutineId id = current_routineid();
        assert(free_list_.find(id) != free_list_.end());
        assert(active_list_.find(id) == active_list_.end());
        assert(!tasks_.empty());

        RoutineCallBack func = std::move(tasks_.front());
        tasks_.pop_front();

        active_list_.insert({id, std::move(free_list_[id])});
        free_list_.erase(id);

        func();
        LOG_INFO << "routine " << id << " func done";
        func = RoutineCallBack();

        free_list_.insert({id, std::move(active_list_[id])});
        active_list_.erase(id);

        current_routine()->Return();
    }
}

}  // namespace runtime

}  // namespace baize
