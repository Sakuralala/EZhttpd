#include <iostream> //for !isOwnerThread
#include <utility>
#include <sys/eventfd.h>
#include <unistd.h>
#include "eventLoop.h"
#include "epoller.h"
#include "event.h"
#include "../bases/thread.h"
#include "../log/logger.h"
namespace event
{
//当前线程的eventloop 初始为null
__thread EventLoop *currentThreadLoop = nullptr;
const int WaitMs = 1000;
EventLoop::EventLoop() : looping_(false), threadID_(bases::currentThreadID()), wakeupFd_(::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)), wakeupEvent_(new Event(wakeupFd_, this)), poller_(new Epoller()), mutex_()
{
    if (currentThreadLoop)
    {
        LOG_FATAL << "Thread:" << threadID_ << " already had a event loop:" << currentThreadLoop;
    }
    currentThreadLoop = this;
    //使能可读事件
    wakeupEvent_->enableRead();
    //设置唤醒回调
    wakeupEvent_->setReadCallback(std::bind(&EventLoop::readHandler, this));
}
EventLoop::~EventLoop()
{
    //其实做不做无所谓，反正eventloop对象都要销毁了
    wakeupEvent_->remove();
    ::close(wakeupFd_);
    currentThreadLoop = nullptr;
}
bool EventLoop::isOwnerThread() const
{
    return threadID_ == bases::currentThreadID();
}
void EventLoop::assertInOwnerThread()
{
    if (!isOwnerThread())
    {
        std::cout << "Current thread:" << bases::currentThreadID() << " does not own the event loop,the owner of the eventloop is thread:" << threadID_ << ".";
        LOG_FATAL << "Current thread:" << bases::currentThreadID() << " does not own the event loop,the owner of the eventloop is thread:" << threadID_ << ".";
    }
}

//当前线程被唤醒事件的回调
void EventLoop::readHandler()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    //TODO:封装底层的一些套接字操作
    if (n != sizeof one)
    {
        LOG_ERROR << "Event loop read " << n << " rather than 8.";
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    //TODO:封装底层的一些套接字操作
    if (n != sizeof one)
    {
        LOG_ERROR << "Event loop write" << n << " rather than 8.";
    }
}
void EventLoop::updateEvent(Event *ev)
{
    //为了保护event的成员
    assertInOwnerThread();
    poller_->updateEvent(ev);
}
TimerKey EventLoop::addTimer(uint64_t secs, typename TimerSet::Callback cb,uint64_t interval)
{
    //无锁，所以需要保证由owner线程调用此函数
    //其他线程通过queueInLoop或runInLoop进行间接调用
    assertInOwnerThread();
    return timerSet_.add(secs, std::move(cb),interval);
}
/*
void EventLoop::addTimer(const Timer &t)
{
}
*/
void EventLoop::delTimer(const TimerKey &tk)
{
    assertInOwnerThread();
    timerSet_.del(tk);
}
//退出循环
void EventLoop::quit()
{
    //使用mutex防止其他线程刚将looping_置为false，owner线程刚好运行到while(looping_)后退出，
    //进一步地使得owner IO线程的threadFunc退出从而导致eventloop对象析构，这样会导致UB;
    //因为quit调用还没结束，其对象eventloop却销毁了
    bases::MutexGuard mg(mutex_);
    if (looping_)
    {
        looping_ = false;
        if (!isOwnerThread())
            wakeup();
    }
}
void EventLoop::loop()
{
    //one loop per thread
    assertInOwnerThread();
    looping_ = true;
    while (looping_)
    {
        int n = poller_->poll(WaitMs, readyList_);
        if (n)
        {
            //NOTE:一系列callback的起点都在此处，为避免race condition,在loop的开头调用了
            //assert,故正常来说在一系列的各个callback中并不需要调用assert，因为它们一般是
            //被动被调用的，即从此处开始被调用;但是对于主动调用callback的情形，还是要根
            //据是否会产生race condition的情况决定要不要assert.
            for (auto eventPtr : readyList_)
                eventPtr->handleEvent();
            readyList_.clear();
        }
        //处理定时器事件
        timerSet_.getExpired();
        //处理完就绪事件后处理用户或其他线程给的任务
        executeCallbacks();
    }
    //保证quit()能够正常退出
    bases::MutexGuard mg(mutex_);
}
//可放心由其他线程调用
void EventLoop::runInLoop(const Callback &cb)
{
    if (isOwnerThread())
        cb();
    else
        queueInLoop(cb);
}
//同上
void EventLoop::queueInLoop(Callback cb)
{
    {
        bases::MutexGuard mg(mutex_);
        callbackList_.push_back(std::move(cb));
    }
    //如果调度当前loop的线程不是其owner线程，由于不知道当前线程是否在运行,以及运行到哪，需要wakeup一下；
    //当其正在处理就绪事件时,有可能会导致owner线程的多余一次醒来；
    //如果是owner线程，那么如果当前正在执行回调，需要wakeup一下；
    if (!isOwnerThread() || isExecutingCallback)
        wakeup();
}

void EventLoop::executeCallbacks()
{
    CallbackList tmp;
    {
        bases::MutexGuard mg(mutex_);
        isExecutingCallback = true;
        callbackList_.swap(tmp);
    }
    for (auto &cb : tmp)
        cb();

    bases::MutexGuard mg(mutex_);
    isExecutingCallback = false;
}
} // namespace event