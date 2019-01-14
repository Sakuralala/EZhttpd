#ifndef __singleton_h
#define __singleton_h
#include "asyncLog.h"
/***
 * 线程安全的单例模式，目前仅支持c++11，因为c++11标准规定了local static variable的初始
 * 化行为，要求编译器必须保证其初始化的线程安全性。
 * **/
namespace bases
{
template <typename T>
class Singleton
{
private:
  Singleton() {}
  ~Singleton() {}

public:
  static T *getInstance()
  {
    static T instance;
    return &instance;
  }
};
//后端日志线程的全特化单例模式;
//TODO:这种做法的话，无法在thread启动的这个过程中进行log
//否则会出现单线程的死锁:外面某个log启动了日志线程到这里的init,这时候pthread_once的锁还没
//释放,而在init中run的内部进入thread.run,在某处进行log时，又会重复上述过程到pthread_once
//然后就死锁了，第一次获取了锁的在等init完成，而第二次尝试获取锁在等锁释放，典型的单线程死锁
template <>
class Singleton<AsyncLog>
{
  public:
    static AsyncLog *getInstance()
    {
        pthread_once(&pOnce_, &Singleton<AsyncLog>::init);
        return instancePtr_;
    }

  private:
    static pthread_once_t pOnce_;
    static AsyncLog *instancePtr_;
    static void init()
    {
        static AsyncLog instance(LogPath);
        instancePtr_ = &instance;
        instance.run();
    }
};
pthread_once_t Singleton<AsyncLog>::pOnce_ = PTHREAD_ONCE_INIT;
AsyncLog* Singleton<AsyncLog>::instancePtr_ = nullptr;
} // namespace bases
#endif // !__singleton_h
