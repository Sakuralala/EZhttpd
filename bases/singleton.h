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
