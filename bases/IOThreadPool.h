#ifndef __iothreadpool_h
#define __iothreadpool_h
#include <vector>
#include <memory>
#include <functional>
#include "mutex.h"
namespace event
{
class IOThread;
class EventLoop;
class IOThreadPool
{
public:
  typedef std::function<void()> Task;
  IOThreadPool();
  //unique_ptr需要看到其内部包裹的类的完整定义才能执行delete，而这里IOThread的定义头文件没
  //有包括在此头文件中(减少编译依赖),所以需要在源文件中写~IOThreadPool
  ~IOThreadPool();
  //启动io线程池
  void run(uint32_t num);
  //停止io线程池
  void stop();
  //主线程给任务
  void put(Task task);
  EventLoop* getNextLoop() const
  {
    if(started_)
      return loops_[next_];
    return nullptr;
  }

private:
  std::vector<std::unique_ptr<IOThread>> IOThreads_;
  std::vector<EventLoop *> loops_;
  const pid_t tid_;
  int next_;
  bool started_;
};
} // namespace event
#endif // !__iothreadpool_h