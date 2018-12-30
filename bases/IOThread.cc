#include <functional>
#include "IOThread.h"
#include "thread.h"
#include "eventLoop.h"
namespace event
{
IOThread::IOThread() : latch_(1), thread_(std::bind(&IOThread::IOThreadFunc, this), "IO Thread"), loop_(nullptr)
{
}

IOThread::~IOThread()
{
    if (loop_)
    {
        //做不做无所谓，反正当前io线程都要析构了
        loop_->quit();
        join();
    }
}

void IOThread::run()
{
    thread_.run();
    //保证子线程已经创建完eventloop
    latch_.wait();
}

void IOThread::join()
{
    if (loop_)
        thread_.join();
}
void IOThread::IOThreadFunc()
{
    //在子线程真正开始执行时才创建eventloop对象
    EventLoop eLoop;
    loop_ = &eLoop;
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
