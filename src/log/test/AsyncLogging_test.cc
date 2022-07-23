#include "log/AsyncLogging.h"

#include <iostream>

#include "log/Logger.h"
#include "thread/Thread.h"
#include "time/Timestamp.h"
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
    g_last_time = Timestamp::now();
    while (1) {
        sleep(1);
        Timestamp current_time(Timestamp::now());
        double sec = elapsedInSecond(current_time, g_last_time);
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
    Thread thread_log([] { log_print(); }, "log");
    thread_log.start();

    AsyncLogging log("AsyncLoggingTest", 1024 * 1024 * 1000);
    Logger::setOutput([&](const char* msg, int len) {
        log.append(msg, len);
        g_writtenbytes += len;
    });
    Logger::setFlush([&] { log.flush(); });
    log.start();

    while (1) {
        LOG_INFO << "hello world" << 123;

        // LOG_INFO << "AsyncLogging start"
        //          << "AsyncLogging start"
        //          << "AsyncLogging start";

        // LOG_INFO << 111111
        //          << 222222;

        // LOG_INFO << "h" << "e" << "l" << "l" << "o"
        //          << "h" << "e" << "l" << "l" << "o";
        // LOG_INFO
        //     <<
        //     "hellohellohellohellohellohellohellohellohellohellohellohello";

        // LOG_INFO << "AsyncLogging start AsyncLogging start AsyncLogging
        // start";
    }
}