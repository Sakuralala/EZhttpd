#include <functional>
#include "IOThread.h"
#include "thread.h"
#include "eventLoop.h"
namespace event
{
IOThread::IOThread() : Thread(std::bind(&IOThread::IOThreadFunc, this), "IO Thread"), loop_(nullptr)
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
    //另外，需要把negSemaphore改为信号量，因为刚开始cnt只减不增，这样会导致后续wait都无效
    latch_.wait();
}
//在存在eventLoop的情况下才能join
void IOThread::join() 
{
    if (loop_)
        Thread::join();
}
void IOThread::IOThreadFunc()
{
    //在子线程真正开始执行时才创建eventloop对象
    EventLoop eLoop;
    loop_ = &eLoop;
    //race condition:子线程先执行到此处时会导致cnt变为-1；
    //解决方法：修改negSemaphore底层wait，在cnt>0时才wait;修改countDown,在cnt<=0时均进行notify;
    latch_.countdown();
    try
    {
        loop_->loop();
    }
    catch (...)
    {
        //TODO:log<<"IO thread loop error."<<endl;
        throw;
    }
    loop_ = nullptr;
}
} // namespace event
