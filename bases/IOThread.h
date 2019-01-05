#ifndef __iothread_h
#define __iothread_h
/**
 *  one loop per thread,即包含一个event loop的thread,即io线程；
 **/
#include "thread.h"

/*
namespace bases
{
class Thread;
class CoutdownLatch;
} // namespace bases
*/
namespace event
{
class EventLoop;
class IOThread : public bases::Thread
{
public:
  IOThread();
  ~IOThread();
  //io线程执行的函数
  void IOThreadFunc();
  //运行IO线程
  void run() override;
  //停止IO线程
  void join() override;
  EventLoop *getLoop() const
  {
    return loop_;
  }

private:
  //这里极其罕见地使用了owning raw pointer,因为决定让loop是一个栈上对象
  EventLoop *loop_;
};
} // namespace event
#endif // !__eventloopthread_h
