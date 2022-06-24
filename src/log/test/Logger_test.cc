#include "log/Logger.h"

#include <assert.h>
#include <stdio.h>

using namespace baize;

int main() {
    LOG_TRACE << "logger test" << 3;
}