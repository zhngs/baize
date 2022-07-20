#include "log/Logger.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>

using namespace baize;

int main()
{
    LOG_TRACE << "logger test" << 3;
    LOG_SYSERR << "test error";
    open("/notexist", 0);
    LOG_SYSERR << "test error2";
}