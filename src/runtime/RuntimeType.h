#ifndef BAIZE_RUNTIMETYPE_H
#define BAIZE_RUNTIMETYPE_H

#include <functional>

namespace baize {

typedef std::function<void()> RoutineCallBack;
typedef std::function<void()> FunctionCallBack;

typedef std::pair<int, int> WaitRequest;
const mode_t WAIT_READ_REQUEST = 1;
const mode_t WAIT_WRITE_REQUEST = 2;

} // namespace baize

#endif // BAIZE_RUNTIMETYPE_H