#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include "IOThreadPool.h"
#include "eventLoop.h"
#include "event.h"
#include "timer.h"
#include "thread.h"
#include "asyncLog.h"
#include "threadPool.h"
using namespace bases;
using namespace event;
using std::cout;
using std::endl;

EventLoop *globalLoop = nullptr;
int *globalFd = nullptr;
void loopFunc()
{
    globalLoop->loop();
}
void timeout()
{
    timeval tv;
    gettimeofday(&tv, nullptr);

    cout << "Thread ID:" << bases::currentThreadID() << ",timeout." << endl;
}
int main()
{

    /*
   int num = 4;
    IOThreadPool iotp;
    iotp.start(num);
    while (num)
    {
        Timer ti1(num);
        ti1.setCallback(&timeout);
        iotp.put(std::bind((void (EventLoop::*)(const Timer&))&EventLoop::addTimer,iotp.getNextLoop(),ti1));
        iotp.put(std::bind((void (EventLoop::*)(uint64_t, typename TimerSet::Callback)) & EventLoop::addTimer, iotp.getNextLoop(), num*2, &timeout));
        num--;
    }
    sleep(10);
    iotp.stop();
    */
    /*
    cout << "thread id:" << bases::currentThreadID() << endl;
    //TODO:多次定时
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    globalFd = &timerfd;
    struct itimerspec tm;
    memset(&tm,0,sizeof tm);
    //首次触发
    tm.it_value.tv_sec = 2;
    //周期性触发
    tm.it_interval.tv_sec = 1;
    IOThread t1;
    t1.run();
    globalLoop = t1.getLoop();
    event::Event ev(timerfd, globalLoop);
    ev.setReadCallback(&timeout);
    //保证对epoll对象的更新由owner线程完成
    globalLoop->runInLoop(std::bind(&event::Event::enableRead,&ev));
    ::timerfd_settime(timerfd,0,&tm,nullptr);
    t1.join();
    ::close(timerfd);
    */
}