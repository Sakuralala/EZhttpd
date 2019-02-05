#ifndef __logfile_h
#define __logfile_h
#include <string>
#include <stdio.h>
#include <memory>
#include "../bases/mutex.h"
/***
 *异步日志后端写操作的封装；包括向文件写、自动创建新日志文件并归档旧文件; 
 * 
 * ***/
namespace bases
{
class AppendFile;
class LogFile
{
  public:
    LogFile(const std::string &name, uint32_t interval=16);
    //必须强制outline  因为unique_ptr必须看到AppendFile的完整定义才知道析构时如何删除
    //而这里使用了前向声明(incompleted-type)
    ~LogFile();
    void append(const char *buf, size_t len);
    void flush();

  private:
    std::string baseName_;
    //刷新写间隔
    uint32_t flushInterval_;
    uint32_t cnt_;
    static const int rollSeconds = 60 * 24 * 24;
    //一个日志文件最大1g
    static const int RollSize = 1024 * 1024 * 1024;
    Mutex mutex_;
    std::unique_ptr<AppendFile> file_;
    //新建一个日志文件
    void rollFile();
    std::string getLogFileName();
};
} // namespace bases
#endif // !__logfile_h
