#ifndef BAIZE_LOGFILE_H
#define BAIZE_LOGFILE_H

#include "util/types.h"

#include <memory>

namespace baize
{

namespace log
{

class AppendFile;

class LogFile //noncopyable
{
public:
    LogFile(const string& basename,
            off_t rollSize,
            int flushInterval = 3,
            int checkEveryN = 1024);
    ~LogFile();
    LogFile(const LogFile&) = delete;
    LogFile& operator=(const LogFile&) = delete;

    void append(const char* logline, int len);
    void flush();
    bool rollFile();

    static string getLogFileName(const string& basename, time_t *now);

private:
    const string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    int count_;

    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    std::unique_ptr<AppendFile> file_;

    const static int kRollPerSeconds_ = 60*60*24;
};
    
} // namespace log
    
} // namespace baize


#endif //BAIZE_LOGFILE_H