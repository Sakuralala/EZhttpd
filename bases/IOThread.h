#ifndef __iothread_h
#define __iothread_h
/**
 *  one loop per thread,即包含一个event loop的thread,即io线程；
 * 
 * 
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
class IOThread
{
public:
  IOThread();
  ~IOThread();
  //io线程执行的函数
  void IOThreadFunc();
  //启动IO线程
  void run();
  //停止IO线程
  void join();
  EventLoop *getLoop() const
  {
    return loop_;
  }

private:
  //为了保证子线程开始运行时eventloop被创建,即在子线程
  //完成eventloop的初始化之前暂时先阻塞主线程
  bases::CountdownLatch latch_;
  bases::Thread thread_;
  //这里极其罕见地使用了owning raw pointer,因为决定让loop是一个栈上对象
  EventLoop *loop_;
};
} // namespace event
#endif // !__eventloopthread_h
