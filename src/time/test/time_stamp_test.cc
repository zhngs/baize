#include "time/time_stamp.h"

#include "log/logger.h"

using namespace baize;

int main()
{
    time::Timestamp now = time::Timestamp::Now();
    LOG_TRACE << now.date();
}