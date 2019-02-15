#include <functional>
#include "IOThread.h"
#include "eventLoop.h"
#include "../bases/thread.h"
#include "../log/logger.h"
namespace event
{
IOThread::IOThread() : Thread(std::bind(&IOThread::IOThreadFunc, this), "IO Thread"), loop_(nullptr), latch__(1)
{
}

IOThread::~IOThread()
{
    if (loop_)
    {
        //做不做无所谓，反正当前io线程都要析构了
        loop_->quit();
        //join();
    }
}
void IOThread::run()
{
    Thread::run();
    //Thread::run()只能保证IOThread至少准备开始执行IOThreadFunc,并不能保证eventloop已经被创建
    //所以主线程需要再进一步等待一下(如果IOThreadFunc里已经countdown了那么主线程不会阻塞)
    //另外，需要把countdownLatch改为信号量，因为刚开始cnt只减不增，这样会导致后续wait都不需要等待

    //后续，想了一下，觉得信号量不太符合这里的意思，最后还是改为了使用两个countdownLatch
    latch__.wait();
}
//在存在eventLoop的情况下才能join
void IOThread::join()
{
    if (loop_)
    {
        //需要先退出循环  不然无法join
        loop_->quit();
        Thread::join();
    }
}
void IOThread::IOThreadFunc()
{
    //在子线程真正开始执行时才创建eventloop对象
    EventLoop eLoop;
    loop_ = &eLoop;
    //race condition:子线程先执行到此处时会导致cnt变为-1；
    //解决方法：修改countdownLatch底层wait，在cnt>0时才wait;修改countDown,在cnt<=0时均进行notify;
    //后续，想了一下，觉得信号量不太符合这里的意思，最后还是改为了使用两个countdownLatch
    latch__.countdown();
    //try
    //{
    loop_->loop();
    //}
    /*
    catch (...)
    {
        LOG_FATAL<<"IO thread loop error.";
    }
    */
    loop_ = nullptr;
}
} // namespace event
