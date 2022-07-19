#include "log/LogFile.h"

#include <unistd.h>

#include "log/AppendFile.h"

using namespace baize;

log::LogFile::LogFile(const string &basename, off_t rollSize, int flushInterval,
                      int checkEveryN)
    : basename_(basename), rollSize_(rollSize), flushInterval_(flushInterval),
      checkEveryN_(checkEveryN), count_(0), startOfPeriod_(0), lastRoll_(0),
      lastFlush_(0) {
  rollFile();
}

log::LogFile::~LogFile() {}

bool log::LogFile::rollFile() {
  time_t now = 0;
  string filename = getLogFileName(basename_, &now);
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

string log::LogFile::getLogFileName(const string &basename, time_t *now) {
  string filename;
  filename.reserve(basename.length() + 64);
  filename += basename;

  char timebuf[32] = "";
  struct tm tm;
  *now = time(NULL);
  gmtime_r(now, &tm); // FIXME: localtime_r ?
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

void log::LogFile::flush() { file_->flush(); }

void log::LogFile::append(const char *logline, int len) {
  file_->append(logline, len);

  if (file_->getWrittenBytes() > rollSize_) {
    rollFile();
  } else {
    ++count_;
    if (count_ >= checkEveryN_) {
      count_ = 0;
      time_t now = ::time(NULL);
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (thisPeriod_ != startOfPeriod_) {
        rollFile();
      } else if (now - lastFlush_ > flushInterval_) {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}