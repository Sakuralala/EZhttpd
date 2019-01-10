#ifndef _condition_h
#define _condition_h
#include "mutex.h"
#include <sys/time.h>
namespace bases
{

class Condition : Nocopyable
{
public:
  explicit Condition(Mutex &m) : mutex_(m)
  {
    pthread_cond_init(&condition_, nullptr);
  }
  ~Condition() { pthread_cond_destroy(&condition_); }

  void wait() { pthread_cond_wait(&condition_, mutex_.getPthreadMutex()); }
  //等待一定时间后退出
  void timedWait(uint32_t secs)
  {
    struct timeval tv;
    struct timespec timeout;
    gettimeofday(&tv, nullptr);
    timeout.tv_sec = tv.tv_sec + secs;
    timeout.tv_nsec = tv.tv_usec * 1000;
    pthread_cond_timedwait(&condition_, mutex_.getPthreadMutex(), &timeout);
  }
  void notify() { pthread_cond_signal(&condition_); }
  void notifyAll() { pthread_cond_broadcast(&condition_); }

private:
  //调用者在外面使用的mutex和condition里面这个应该是一个东西，所以这里还是个引用
  //不然有可能会死锁，考虑下这种情况:如果wait线程拿到锁1之后,发现条件不满足，那么
  //调用wait阻塞，然后假设现在来了数据，signal线程想要调用notify,但是之前需要先获取
  //锁1，而锁1又被wait线程拿着，wait线程又等着signal线程唤醒它以便处理完数据后释放锁1...
  Mutex &mutex_;
  pthread_cond_t condition_;
};
} // namespace bases
#endif