#ifndef _mutex_h
#define _mutex_h
#include "nocopyable.h"
#include <pthread.h>
namespace bases
{
class MutexGuard;

class Mutex : Nocopyable
{
  friend MutexGuard;

public:
  Mutex() { pthread_mutex_init(&mutex_, nullptr); }
  ~Mutex() { pthread_mutex_destroy(&mutex_); }
  pthread_mutex_t *getPthreadMutex() { return &mutex_; }

private:
  pthread_mutex_t mutex_;
  void lock() { pthread_mutex_lock(&mutex_); }
  void unlock() { pthread_mutex_unlock(&mutex_); }
};

class MutexGuard : Nocopyable
{
public:
  explicit MutexGuard(Mutex &m) : mutex_(m) { mutex_.lock(); }
  ~MutexGuard() { mutex_.unlock(); }

private:
  // mutexguard用于保护临界区
  // 那么它就不能作为其他类的成员(不然的话临界区就和类对象的生命一样长了)
  //故使用引用 其他类拥有的应该是mutex而不是mutexguard
  Mutex &mutex_;
};
}

#define MutexGuard(x) static_assert(false,"Forgot to name mutex guard.")
#endif