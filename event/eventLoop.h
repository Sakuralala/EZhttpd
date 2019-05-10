#ifndef __eventloop_h
#define __eventloop_h
/**
 *io线程的主循环，正常情况下就是在使用多路复用器(epoll)在等待就绪事件，有就绪事件后就依次调 
 *用各个回调；它由某个线程创建，并且从始至终属于创建它的那个线程，且一个线程只能有一个event
 *loop;
 *是否使用锁的总体原则:需要跨线程调用的函数，若存在同时读写成员的可能性就要加锁;不能跨线程使用
 *的函数，必须在开头检查是否为owner线程调用，否则直接报错退出；另外，锁的持有者应该是成员对象
 *的拥有者而不是调用者；     
**/
#include <memory>
#include <vector>
#include <functional>
#include "../bases/mutex.h"
#include "timer.h"

namespace event
{
//避免包含头文件，减小编译依赖
class Event;
class Epoller;
class EventLoop
{
public:
  typedef std::function<void()> Callback;
  //只要做好Event类的owner类的生命期管理，Event对象的生命期不成问题
  typedef std::vector<Event *> EventList;
  typedef std::vector<Callback> CallbackList;
  EventLoop();
  ~EventLoop();
  //主循环 仅能由owner线程调用
  void loop();
  //退出循环  可能由其他线程调用，所以需要加锁
  void quit();
  //一个IO线程对应一个event loop
  bool isOwnerThread() const;
  //确保当前eventloop在创建它的io线程中使用
  void assertInOwnerThread();
  //用于在IO线程中执行用户回调;或者将用户回调放到另一个IO线程中执行
  //这在主线程分配新连接给IO线程时很有用，大概来说让某个IO线程执行新连接建立后的回调
  //这个回调内部会将新连接相关信息加入到IO线程的epoll对象中
  //可由非owner线程调用 内部会根据当前线程是否为owner线程选择是否调用queueInLoop
  void runInLoop(const Callback &cb);
  //用于将用户回调加入回调列表中，并在必要时(不在对应的IO线程，或者正在执行用户回调)唤醒IO线程
  //可由非owner线程调用
  void queueInLoop(Callback cb);
  //在当前事件循环的epoller中更新需要监控的事件,由于epoll对象内部有锁保护，所以这个函数可以跨线程且不需要加锁
  void updateEvent(Event *ev);
  //定时器事件添加
  //TODO:存在重载函数的话，bind时需要显式将对应的绑定函数转换为对应要绑定的函数的函数类型的函数指针，代码比较丑陋
  //FIXED:去掉一个
  TimerKey addTimer(uint64_t secs,typename TimerSet::Callback cb,uint64_t interval=1);
  //void addTimer(const Timer &t);
  void delTimer(const TimerKey &tk);

private:
  //仅在loop()及quit()中可以读写,在quit()中读写需加锁(因为可能会跨线程)
  bool looping_;
  //只读 不需要锁保护 线程安全
  const pid_t threadID_;
  //使用eventfd从而在其他的线程中唤醒当前线程 一个用途就是有新连接到来时主线程通过某种方式
  //将建立的新连接分配到某个io线程中 那么此时需要对io线程进行唤醒操作(因为有可能阻塞在epoll上)
  //只读 不需要锁保护 线程安全
  int wakeupFd_;
  //唤醒后需要调用相应的事件回调 具体来说当这个只是用来监控其他线程唤醒当前IO线程的事件，即读事件
  //由于相互引用，所以此处用指针
  //以下两个变量由owner线程独占，不需要考虑线程安全问题
  std::unique_ptr<Event> wakeupEvent_;
  //降低头文件依赖，使用指针
  std::unique_ptr<Epoller> poller_;
  //就绪的事件列表
  EventList readyList_;
  //以下两个变量可能会在非owner线程中读，在owner线程写，故需要锁保护
  //是否正在执行callbackList_
  bool isExecutingCallback;
  //回调列表
  CallbackList callbackList_;
  //用于保护callbackList_及isExecutingCallback,由于其有可能在其他线程中被读写，所以需要进行加锁操作
  bases::Mutex mutex_;
  //定时器集合
  TimerSet timerSet_;
  /*************************以下几个为仅供内部调用的函数************************************/
  //执行一些自定义的回调  
  void executeCallbacks();
  //被唤醒后的当前IO线程调用的回调
  void readHandler();
  //唤醒当前EventLoop所对应的线程,可由非owner线程调用
  void wakeup();
};
extern __thread EventLoop *currentThreadLoop;
} // namespace event
#endif // !__eventloop_h