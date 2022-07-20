#include "time/Timestamp.h"

#include "log/Logger.h"

using namespace baize;

int main()
{
    time::Timestamp now = time::Timestamp::now();
    LOG_TRACE << now.toFormatString();
}