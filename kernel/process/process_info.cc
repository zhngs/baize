#include "process/process_info.h"

namespace baize
{
namespace process
{

pid_t pid() { return ::getpid(); }

}  // namespace process

}  // namespace baize
