#ifndef __timer_h
#define __timer_h
/***
 * timer的实现的重点在于保存其的数据结构，有这几个要求：快速找到所有到期的timer节点；快速添
 * 加和删除到期的timer节点。可以考虑的东西有：二叉堆、红黑树。这里由于stl的priority_queue不
 * 能随机删除堆中的某个元素，故选择了以红黑树为底层结构的set。
 * 获取当前时间的底层调用是get_timeofday(),由于多次调用会加大系统调用的开销，可以考虑使用缓
 * 存时间来降低这部分开销，可以参考nginx的time.c的实现。其底层实现的更高性能的和读写锁相同效
 * 果的时间槽机制也可以借鉴一下，用于减小同时读写加锁(nginx同时读写不需要加锁)所带来的开销。
 * UPDATE:2.6内核之后的get_timeofday是虚拟系统调用，开销和普通函数调用差不多，无需做时间的缓
 * 存；     
 **/
#include <unistd.h> //for uint64_t
#include <map>
//#include <set>
#include <memory>
#include <string>
#include <functional> //for std::function
#include "../bases/nocopyable.h"
namespace event
{
//定时器节点，内部类，只能通过TimerSet进行相关调用,表示的是从1970.01.01开始到现在所经过的毫秒数+一个超时回调
class Timer
{
  friend class TimerSet;

public:
  typedef std::function<void()> TimeoutCallback;
  Timer() {}
  explicit Timer(uint64_t secs, uint64_t repeats = 1);
  explicit Timer(uint64_t secs, TimeoutCallback cb, uint64_t repeats = 1);
  ~Timer() = default;

  uint64_t getTime() const
  {
    return milliSeconds_;
  }
  uint64_t getSeconds() const
  {
    return seconds_;
  }
  uint64_t getRepeatTimes() const
  {
    return repeatTimes_;
  }
  void decreaseRepeatTimes()
  {
    if (repeatTimes_ != 0)
      --repeatTimes_;
  }
  void setTimer(uint64_t secs);
  //重新设置定时器
  void resetTimer();
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
  //定时器的重复次数 默认为1,0表示无穷
  uint64_t repeatTimes_;
  //最初设置的超时时间
  uint64_t seconds_;
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
//FIXME:此做法不妥，因为需要保证插入timerSet时使用的timer的生命周期和timerSet一致(不然timer被析构了再解引就是UB了)
//FIXED:解决方案，timer的添加改为内部进行，Timer改为内部类，只提供一个addTimer接口给外部使用，
//并返回一个TimerKey,实际就是内部Timer的地址，内部创建的Timer使用堆上内存；另外，TimerSet的映
//射表改为单一的set即可；
class TimerKey
{
  friend class TimerSet;

public:
  TimerKey(const Timer *ptr) : timerPtr_(ptr) {}
  ~TimerKey() = default;
  uint64_t getTime() const
  {
    return timerPtr_->getTime();
  }
  std::string format() const
  {
    if (timerPtr_)
      return timerPtr_->format();
    return "error time.";
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
  TimerKey add(uint64_t sec, Callback cb, uint64_t interval = 1);
  //void add(const Timer &t);
  //NOTE:需要一个删除函数，处理这种情况:对端在超时时限内又发送了新的数据，需要重置定时器
  //所以需要一个独一无二的标示符来代表每个定时器，此处使用了timer的地址
  void del(const TimerKey &);
  uint64_t getNext() const;

private:
  //为什么不使用hashMap的主要原因是需要保证timer的有序性以便能够getExpired时快速删除超时的定时器
  //std::map<TimerKey, Timer> timerSet_;
  typedef std::unique_ptr<Timer> TimerPtr;
  //用TimerPtr作为Val的缺点在于 每次getExpired的时候如何处理多次超时的定时器的问题，因为调用者
  //手中持有的是最初的TimerKey,这边timerSet_直接删除再插入dAljfda
  std::map<TimerKey, TimerPtr> timerSet_;
};
} // namespace event
#endif // !__timer_h
