#include "log/async_logging.h"

#include <iostream>

#include "log/logger.h"
#include "thread/thread.h"
#include "time/time_stamp.h"
#include "unistd.h"

using namespace baize;
using namespace baize::log;
using namespace baize::time;
using namespace baize::thread;

uint64_t g_writtenbytes = 0;
uint64_t g_writtenbytes_last = 0;
Timestamp g_last_time;

void log_print()
{
    g_last_time = Timestamp::Now();
    while (1) {
        sleep(1);
        Timestamp current_time(Timestamp::Now());
        double sec = ElapsedInSecond(current_time, g_last_time);
        double write_bytes =
            static_cast<double>(g_writtenbytes - g_writtenbytes_last);
        double speed = write_bytes / sec / 1024 / 1024;

        std::cout << "logger write speed " << speed << " MiB/s\n";

        g_writtenbytes_last = g_writtenbytes;
        g_last_time = current_time;
    }
}

int main()
{
    Thread thread_log("log", [] { log_print(); });
    thread_log.Start();

    AsyncLogging log("AsyncLoggingTest", 1024 * 1024 * 1000);
    Logger::set_output([&](const char* msg, int len) {
        log.Append(msg, len);
        g_writtenbytes += len;
    });
    Logger::set_flush([&] { log.Flush(); });
    log.Start();

    while (1) {
        LOG_INFO << "hello world" << 123;
    }
}