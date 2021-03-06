#include <sys/time.h>
#include <unistd.h> //for access
#include <string.h>
#include "logFile.h"
#include "logger.h"
#include "../bases/fileUtil.h"
namespace bases
{
LogFile::LogFile(const std::string &name, uint32_t interval) : baseName_(name), flushInterval_(interval), cnt_(0), mutex_()
{
    rollFile();
}
LogFile::~LogFile() = default;
void LogFile::append(const char *buf, size_t len)
{
    //因为目前只有一个日志线程，所以用不用mutex其实一样
    MutexGuard mg(mutex_);
    //防止中途把日志文件删了
    //if (access(curFileName_.c_str(), F_OK))
    //    rollFile();
    file_->append(buf, len);
    if (++cnt_ > flushInterval_)
        file_->flush();
    //超大小 roll FIXME:超大小不roll???
    //FIXED:超大小不roll的原因是由于测试时写日志太快了，时间的秒数没变，导致文件名没变。。。
    //其实不算bug,正常来说roll size设的大一点，那么时间肯定是变的；
    //为了尽可能避免这种情况，在命名时最后再加一个微秒值，这样应该就没啥问题了
    if (file_->getCurrentSize() > RollSize)
    {
        LOG_INFO << "Log file size limited,roll.";
        //不需要flush 因为后面AppendFile的析构会fclose，这会导致用户态缓冲区的刷新(到内核高速缓存)
        rollFile();
    }
}
void LogFile::rollFile()
{
    curFileName_ = getLogFileName();
    file_.reset(new AppendFile(curFileName_));
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
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d-%02d-%02d-%02d-%ld",
             formatedTime.tm_year + 1900, formatedTime.tm_mon + 1, formatedTime.tm_mday,
             formatedTime.tm_hour, formatedTime.tm_min, formatedTime.tm_sec, tv.tv_usec);
    return baseName_ + buf + ".log";
}
} // namespace bases