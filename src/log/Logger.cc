#include "log/Logger.h"

#include <stdio.h>

using namespace baize;

log::Logger::Logger(const char* filename, int line, const char* func)
  : filename_(filename),
    line_(line),
    func_(func)
{

}

log::Logger::~Logger()
{
    printf("%s %d %s", filename_, line_, func_);
}