#include <sys/time.h>
#include <string.h>
#include "logFile.h"
#include "fileUtil.h"
namespace bases
{
LogFile::LogFile(const std::string &name, uint32_t interval) : baseName_(name), flushInterval_(interval), cnt_(0), mutex_()
{
    rollFile();
}
LogFile::~LogFile() = default;
void LogFile::append(const char *buf, size_t len)
{
    MutexGuard mg(mutex_);
    file_->append(buf, len);
    if (++cnt_ > flushInterval_)
        file_->flush();
    //超大小 roll
    if (file_->getCurrentSize() > RollSize)
        rollFile();
}
void LogFile::rollFile()
{
    std::string name = getLogFileName();
    file_.reset(new AppendFile(name));
}
void LogFile::flush()
{
    MutexGuard mg(mutex_);
    file_->flush();
}
std::string LogFile::getLogFileName()
{
    char buf[64];
    memset(buf, 0, sizeof(char) * 64);
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm formatedTime;
    gmtime_r(&tv.tv_sec, &formatedTime);
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d-%02d-%02d-%02d",
             formatedTime.tm_year + 1900, formatedTime.tm_mon + 1, formatedTime.tm_mday,
             formatedTime.tm_hour, formatedTime.tm_min, formatedTime.tm_sec);
    return baseName_ + buf + ".log";
}
} // namespace bases