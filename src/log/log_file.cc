#include "log/log_file.h"

#include <unistd.h>

namespace baize
{

namespace log
{

LogFile::LogFile(const string& basename,
                 off_t rollSize,
                 int flushInterval,
                 int checkEveryN)
  : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    count_(0),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0)
{
    RollFile();
}

LogFile::~LogFile() {}

bool LogFile::RollFile()
{
    time_t now = 0;
    string filename = log_file_name(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_ = std::make_unique<AppendFile>(filename.c_str());
        return true;
    }
    return false;
}

string LogFile::log_file_name(const string& basename, time_t* now)
{
    string filename;
    filename.reserve(basename.length() + 64);
    filename += basename;

    char timebuf[32] = "";
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm);  // FIXME: localtime_r ?
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm);
    filename += timebuf;

    char hostname[256] = "";
    if (::gethostname(hostname, sizeof(hostname)) != 0) {
        snprintf(hostname, sizeof(hostname), "unknownhost");
    }
    filename += hostname;

    char pidbuf[32] = "";
    snprintf(pidbuf, sizeof(pidbuf), ".%d", ::getpid());
    filename += pidbuf;

    filename += ".log";

    return filename;
}

void LogFile::Flush() { file_->Flush(); }

void LogFile::Append(const char* logline, int len)
{
    file_->Append(logline, len);

    if (file_->written() > rollSize_) {
        RollFile();
    } else {
        ++count_;
        if (count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod_ != startOfPeriod_) {
                RollFile();
            } else if (now - lastFlush_ > flushInterval_) {
                lastFlush_ = now;
                file_->Flush();
            }
        }
    }
}

}  // namespace log

}  // namespace baize