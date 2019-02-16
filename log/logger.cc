#include <stdlib.h>
#include "logger.h"
#include "asyncLog.h"
#include "../bases/singleton.h"
#include "../bases/thread.h"
#include "../event/timer.h"
namespace bases
{
LogLevel globalLevel = LogLevel::INFO;
static const char *levelStr[] = {"DEBUG", "INFO", "NOTICE", "WARN", "ERROR", "FATAL"};
Logger::Logger(const char *full_file_path, int line, LogLevel level) : sourceFile_(full_file_path), line_(line), level_(level)
{
    stream_ << formatTime() << " " << currentThreadID() << " " << levelStr[level_] << " ";
}
Logger::~Logger()
{
    stream_ << "    --" << sourceFile_ << " line:" << line_ << "\n";
    auto &buffer = stream_.getBuffer();
    Singleton<AsyncLog>::getInstance()->append(buffer.begin(), buffer.length());
    if (level_ == FATAL)
        abort();
}
std::string Logger::formatTime()
{
    return event::Timer::now().format();
}
} // namespace bases