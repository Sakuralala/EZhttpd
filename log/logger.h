#ifndef __logger_h
#define __logger_h
#include <string>
#include "logStream.h"
namespace bases
{
typedef enum
{
    DEBUG,
    INFO,
    NOTICE,
    WARN,
    ERROR,
    FATAL
} LogLevel;
//这个代表的是当前的全局的级别
extern LogLevel globalLevel;
class Logger
{
  public:
    Logger(const char *full_file_path, int line, LogLevel level);
    ~Logger();
    LogStream &stream()
    {
        return stream_;
    }
    static LogLevel getLevel()
    {
        return globalLevel;
    }
    static void setLevel(LogLevel l)
    {
        globalLevel = l;
    }

  private:
    LogStream stream_;
    //这个代表的是当前logger的级别
    LogLevel level_;
    const char *sourceFile_;
    int line_;
    std::string formatTime();
};
} // namespace bases
#define LOG_DEBUG                                            \
    if (bases::Logger::getLevel() <= bases::LogLevel::DEBUG) \
    bases::Logger(__FILE__, __LINE__, bases::LogLevel::DEBUG).stream()
#define LOG_INFO                                            \
    if (bases::Logger::getLevel() <= bases::LogLevel::INFO) \
    bases::Logger(__FILE__, __LINE__, bases::LogLevel::INFO).stream()
#define LOG_NOTICE                                            \
    if (bases::Logger::getLevel() <= bases::LogLevel::NOTICE) \
    bases::Logger(__FILE__, __LINE__, bases::LogLevel::NOTICE).stream()
#define LOG_WARN                                            \
    if (bases::Logger::getLevel() <= bases::LogLevel::WARN) \
    bases::Logger(__FILE__, __LINE__, bases::LogLevel::WARN).stream()
#define LOG_ERROR                                            \
    if (bases::Logger::getLevel() <= bases::LogLevel::ERROR) \
    bases::Logger(__FILE__, __LINE__, bases::LogLevel::ERROR).stream()
#define LOG_FATAL                                            \
    if (bases::Logger::getLevel() <= bases::LogLevel::FATAL) \
    bases::Logger(__FILE__, __LINE__, bases::LogLevel::FATAL).stream()
#endif //  __logger_h!