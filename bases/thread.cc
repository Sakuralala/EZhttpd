#include <iostream>
//for syscall
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h> //for snprintf
//#include <utility>
#include "../log/logger.h"
#include "thread.h"
using std::cout;
using std::endl;
#define gettid() syscall(SYS_gettid) // for gettid
namespace bases
{
//缓存当前线程的id
__thread pid_t threadID = 0;
__thread int threadIDLength = 0;
//预先格式化线程id的字符串，以便log时使用
//INT_MAX为10位;
__thread char threadIDString[11];
pid_t currentThreadID()
{
    if (!threadID)
    {
        threadID = gettid();
        threadIDLength = snprintf(threadIDString, sizeof threadIDString, "%5d", threadID);
    }

    return threadID;
}
//子线程的运行函数wrapper
//说白了这个wrapper的作用只是为了进行一次系统调用缓存一下当前线程的id
void *threadFuncWrapper(void *args)
{
    Thread *t = static_cast<Thread *>(args);
    t->tid_ = currentThreadID();
    //std::cout << "Sub thread " << t->tid_ << " created,ready to run." << std::endl;
    //一个隐蔽的错误，LOG_INFO不能在这个函数里出现
    t->latch_.countdown();
    //try
    //{
        t->func_();
    //}
    /*
    catch (...)
    {
        throw;
    }
    */
    return nullptr;
}
Thread::Thread(const threadFunc &tf, const std::string &name)
    : tid_(0), pthreadID_(0), running_(false), joined_(false), name_(name), latch_(1),
      func_(tf) {}

Thread::Thread(threadFunc &&tf, const std::string &name)
    : tid_(0), pthreadID_(0), running_(false), joined_(false), name_(name), latch_(1),
      func_(std::move(tf)) {}

void Thread::run()
{
    if (!running_)
    {
        running_ = true;
        if (pthread_create(&pthreadID_, nullptr, &threadFuncWrapper, this))
        {
            //LOG_FATAL << "Create thread failed.";
            running_ = false;
        }
        else
        {
            //note:此处并不会丢失信号，因为countDownLatch把信号通过计数的方式保存了；
            latch_.wait();
            // std::cout << "thread " << tid_ << "started." << std::endl;
            //FIXME:死锁了这里 单个线程自身死锁了 尝试获取pthread_once中的锁两次
            //FIXED:这没办法，毕竟log线程也是thread，解决办法就是线程启动的流程内不进行log
            //因为也不是什么有价值的信息；
            //LOG_INFO << "Sub thread " << tid_ << " created, ready to run.";
        }
    }
}

void Thread::join()
{
    if (running_ && !joined_)
    {
        pthread_join(pthreadID_, nullptr);
        joined_ = true;
        running_ = false;
    }
}

void Thread::detach()
{
    if (running_ && !joined_)
    {
        pthread_detach(pthreadID_);
    }
}

Thread::~Thread()
{
    join();
}
} // namespace bases
