#include <iostream>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>
#include "IOThread.h"
#include "eventLoop.h"
#include "event.h"
#include "timer.h"
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
    /*
    uint64_t one = 0;
    if(::read(*globalFd,&one,sizeof one)!=sizeof one)
        throw;
        */
        
    cout << "Thread ID:" << bases::currentThreadID() << ",timeout." << endl;
    globalLoop->quit();
}
int main()
{
    Timer ti1(2);
    ti1.setCallback(&timeout);
    IOThread t1;
    t1.run();
    globalLoop = t1.getLoop();
    globalLoop->runInLoop(std::bind((void(EventLoop::*)(const Timer&))&EventLoop::addTimer,globalLoop,ti1));
    t1.join();
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