#ifndef __timer_h
#define __timer_h
/***
 * timer的实现的重点在于保存其的数据结构，有这几个要求：快速找到所有到期的timer节点；快速添
 * 加和删除到期的timer节点。可以考虑的东西有：二叉堆、红黑树。这里由于stl的priority_queue不
 * 能随机删除堆中的某个元素，故选择了以红黑树为底层结构的set。
 * 获取当前时间的底层调用是get_timeofday(),由于多次调用会加大系统调用的开销，可以考虑使用缓
 * 存时间来降低这部分开销，可以参考nginx的time.c的实现。其底层实现的更高性能的和读写锁相同效
 * 果的时间槽机制也可以借鉴一下，用于减小同时读写加锁(nginx同时读写不需要加锁)所带来的开销。
 *      
 **/
#include <unistd.h> //for uint64_t
#include <map>
#include <string>
#include <functional> //for std::function
#include "../bases/nocopyable.h"
namespace event
{
//定时器节点，内部类，表示的是从1970.01.01开始到现在所经过的毫秒数+一个超时回调
class Timer
{
public:
  typedef std::function<void()> TimeoutCallback;
  Timer(){}
  explicit Timer(uint64_t secs);
  explicit Timer(uint64_t secs, TimeoutCallback cb);
  ~Timer() = default;

  uint64_t getTime() const
  {
    return milliSeconds_;
  }
  void setTime(uint64_t secs);
  void setCallback(TimeoutCallback cb)
  {
    timeoutCallback_ = std::move(cb);
  }
  TimeoutCallback getCallback() const
  {
    return timeoutCallback_;
  }
  void timeout();
  //返回格式化后的时间
  std::string format() const;
  static std::string format(uint64_t millisecs);
  static Timer now();

private:
  //从1970.01.01开始到现在所经过的毫秒数
  uint64_t milliSeconds_;
  TimeoutCallback timeoutCallback_;
};
//用inline防止报重复定义的错误(因为有多个源文件包含此头文件)
inline bool operator<(const Timer &t1, const Timer &t2)
{
  return t1.getTime() < t2.getTime();
}
inline bool operator==(const Timer &t1, const Timer &t2)
{
  return t1.getTime() == t2.getTime();
}
inline uint64_t operator-(const Timer &t1, const Timer &t2)
{
  auto ms1 = t1.getTime(), ms2 = t2.getTime();
  return ms1 < ms2 ? 0 : (ms1 - ms2) / 1000;
}
//由于map的操作(默认排序、upper/lower_bound)都是按key的排序来进行比较的，所以为了方便，此处直接包装一个TimerKey,内部就是一个Timer*,
//只不过重载了<;
class TimerKey  
{
public:
  TimerKey(const Timer *ptr) : timerPtr_(ptr) {}
  ~TimerKey() = default;
  uint64_t getTime() const
  {
    return timerPtr_->getTime();
  }

private:
  const Timer *timerPtr_;
};
inline bool operator<(const TimerKey &tk1, const TimerKey &tk2)
{
  return tk1.getTime() < tk2.getTime();
}
class TimerSet : Nocopyable
{
public:
  typedef typename Timer::TimeoutCallback Callback;
  TimerSet() = default;
  ~TimerSet() = default;
  void getExpired();
  //void add(uint64_t sec, Callback cb);
  void add(const Timer &t);
  //NOTE:需要一个删除函数，处理这种情况:对端在超时时限内又发送了新的数据，需要重置定时器
  //所以需要一个独一无二的标示符来代表每个定时器，此处使用了timer的地址
  void del(const Timer &);
  uint64_t getNext() const;

private:
  //为什么不使用hashMap的主要原因是需要保证timer的有序性以便能够getExpired时快速删除超时的定时器
  std::map<TimerKey, Timer> timerSet_;
};
} // namespace event
#endif // !__timer_h
