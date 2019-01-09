#ifndef __asynclog_h
#define __asynclog_h
/**
 *日志线程，负责接收前端输出的消息，并且在后端将日志缓冲区输出到文件;这是外部使用的接口；
 *
 * **/
#include <vector>
#include <memory>
#include <functional>
#include "thread.h"
#include "mutex.h"
#include "countdownLatch.h"
#include "condition.h"

namespace bases
{
class AsyncLog
{
  public:
  AsyncLog():thread_(std::bind(&AsyncLog::writeToFile,this),"Log thread"),latch_(1),mutex_(),cond_(mutex_){}
    void run();
    void stop();
    //前端写消息到缓冲区
    void append();
    //后端把缓冲区中的内容写入文件
    void writeToFile();

  private:
    Thread thread_;
    //让日志线程能够开始执行
    CountdownLatch latch_;
    Mutex mutex_;
    Condition cond_;
};
} // namespace bases
#endif // !__asynclog_h