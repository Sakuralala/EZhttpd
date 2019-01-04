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
#include <set>
#include <string>
#include <functional> //for std::function
#include "nocopyable.h"
namespace event
{
//定时器节点，内部类，表示的是从1970.01.01开始到现在所经过的毫秒数+一个超时回调
class Timer
{
public:
  typedef std::function<void()> TimeoutCallback;
  explicit Timer(uint64_t secs);
  explicit Timer(uint64_t secs, TimeoutCallback cb);
  ~Timer() = default;
  uint64_t getTime() const
  {
    return microSeconds_;
  }
  void setCallback(TimeoutCallback cb)
  {
    timeoutCallback_ = cb;
  }
  TimeoutCallback getCallback() const
  {
    return timeoutCallback_;
  }
  void timeout()
  {
    if(timeoutCallback_)
      timeoutCallback_();
  }
  //返回格式化后的时间
  std::string format();
  static Timer now();

private:
  //从1970.01.01开始到现在所经过的毫秒数
  uint64_t microSeconds_;
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

class TimerSet : Nocopyable
{
public:
  typedef typename Timer::TimeoutCallback Callback;
  TimerSet() = default;
  ~TimerSet() = default;
  void getExpired();
  void add(uint64_t sec,Callback cb);
  void add(const Timer &t);
  //void del();
  uint64_t getNext() const;

private:
  std::multiset<Timer> timerSet_;
};
} // namespace event
#endif // !__timer_h
