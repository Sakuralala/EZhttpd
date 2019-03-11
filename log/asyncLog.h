#ifndef __asynclog_h
#define __asynclog_h
/**
 *日志线程，负责接收前端输出的消息，并且在后端将日志缓冲区输出到文件;这是外部使用的接口；
 *
 * **/
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include "../bases/thread.h"
#include "../bases/mutex.h"
#include "../bases/countdownLatch.h"
#include "../bases/condition.h"
#include "../bases/buffer.h"

namespace bases
{
//TODO:改成读取配置文件配置项的形式，或者是读取命令行参数的形式
extern std::string LogPath;
class AsyncLog
{
public:
  typedef FixedLengthBuffer<LargeBufferLength> LogBuffer;
  typedef std::unique_ptr<LogBuffer> LogBufferPtr;
  typedef std::vector<LogBufferPtr> BufferVector;
  AsyncLog(const std::string &pathname);
  ~AsyncLog();
  void run();
  void stop();
  //前端写消息到缓冲区
  bool append(const char *message, size_t length);
  //后端把缓冲区中的内容写入文件
  void writeToFile();

private:
  std::string pathName_;
  Thread thread_;
  //让日志线程能够开始执行
  CountdownLatch latch_;
  Mutex mutex_;
  //用于定时将缓冲区中的日志写入到文件中
  Condition cond_;
  bool looping_;
  LogBufferPtr current_;
  //流动式缓冲区的设计:
  //总的缓冲区的个数在一开始就确定了，不可更改;
  //存在三个队列，一个是空buffer队列empty_，另一个是满buffer队列full_，这两个前后端都可以访问；
  //最后一个队列为满buffer队列的copy:fullCopy,只由后端使用，作用是快速和满buffer队列进行交换从而减小临界区(两个队列的容量是一样的)；
  //大致的工作流程如下:
  //前端只往current_中写数据，满了就将其加入到full_中，并尝试从empty_中拿一块新的，没有则直接返回;
  //后端将fullCopy和full_进行交换，然后就可以离开临界区(然后前端就可以继续写了),开始将fullCopy(实际上是full_)中的数据写入文件，写入完成之后再次进入临界区，这次是为了把fullCopy中的buffer加入到empty中，以便循环利用
  //按上述方式，大概就形成了一个buffer流动的循环
  //备用的buffer块
  BufferVector empty_;
  //写满了的buffer块
  BufferVector full_;
  //满的buffer块和备用buffer块之和的最大值
  static const int MaxNumber = 8;
  static const int DefaultWriteSeconds = 5;
};
} // namespace bases
#endif // !__asynclog_h