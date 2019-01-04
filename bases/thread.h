/**
 * pthread的封装，Thread对象的生命期可能和其对应的子线程一致(程序退出析构Thread,对应的子线程也会
 * 被join);也可能长于其对应的子线程(对应的子线程由于某种原因挂了，需要拉起新的子线程);
 **/
#ifndef _thread_h
#define _thread_h
#include <string>
#include <functional>
#include "negSemaphore.h"
namespace bases
{
extern __thread int threadID;
pid_t currentThreadID();
class Thread : Nocopyable
{
public:
  typedef std::function<void()> threadFunc;
  explicit Thread(const threadFunc &tf, const std::string &);
  explicit Thread(threadFunc &&tf, const std::string &);
  //由IOThread继承  Thread需要一个虚析构函数
  virtual ~Thread();
  bool isRunning() const { return running_; }
  bool isJoined() const { return joined_; }
  // glibc未对gettid做一个wrapper,所以需要直接调用syscall
  //所以需要在外面包一个wrapper以在进入用户回调之前调用一次gettid并缓存
  pid_t getTid() const { return tid_; }
  virtual void run();
  virtual void detach();
  virtual void join();

protected:
  //子线程是否初始化完成 为了让子类能够调用，使用protected
  NegSemaphore latch_;

private:
  //用户回调的wrapper
  //用friend的原因是为了符合pthread_create的call signature,及需要在threadFunWrapper中改变Thread实例的成员(tid)
  //注意循环应该由线程池来包，因为就线程本身而言它并不知道taskqueue的存在，
  //它的任务就是执行threadFuncWrapper
  friend void *threadFuncWrapper(void *);

  pid_t tid_;
  pthread_t pthreadID_;
  bool running_;
  bool joined_;
  std::string name_;
  threadFunc func_;
};
} // namespace bases
#endif // !_thread_h