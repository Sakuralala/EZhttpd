#ifndef _threadpool_h
#define _threadpool_h
//#include "thread.h"
#include <deque>
#include <vector>
#include <memory>
#include <iostream>
namespace bases
{
typedef enum
{
  Immediately,
  Graceful
} ExitMode;
class Thread;
class ThreadPool : Nocopyable
{
public:
  typedef std::function<void()> Task;
  //TODO:任务队列的最大值有待商榷。
  explicit ThreadPool(int qSize = 1024);
  ~ThreadPool();
  void run(int numThreads);
  void stop(ExitMode mode = Immediately);
  Task get();
  //正如c++ primer所讲的 使用一个函数实现copy/move两种功能
  //具体来说 如果传的是左值，那么会先调用copy ctor来构造形参task
  //然后这个task在函数内部就可以任我们摆布了；(1 copy+1 move)
  //如果传的是右值，那么就是:2 move ctor 总之内部可以任意调用std::move
  void put(Task task);
  //子线程运行的循环
  void threadFunction();

private:
  bool running_;
  ExitMode eMode_;
  int maxTaskNum_;
  Mutex mutex_;
  Condition notEmpty_;
  Condition notFull_;
  std::deque<Task> taskQueue_;
  //这里需要使用unique_ptr来把thread包起来 理由:thread是nocopyable的且nomoveable的,而vector保存的东西至少要是moveable的(归根结底还是由于vector的动态扩容操作,导致需要将原空间的内容copy/move到新的空间里)，所以需要包一下；另外，使用unique_ptr的话，就不需要手动调用Thread类的dtor了
  std::vector<std::unique_ptr<Thread>> threads_;
};
} // namespace bases

#endif // !1_threadpool_h