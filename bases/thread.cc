#include <iostream>
//for syscall
#include <sys/syscall.h>
#include <unistd.h>
//#include <utility>
#include "thread.h"
using std::cout;
using std::endl;
#define gettid() syscall(SYS_gettid) // for gettid
namespace bases
{
//缓存当前线程的id
__thread int threadID = 0;
pid_t currentThreadID()
{
    if(!threadID)
        threadID = gettid();
    return threadID;
}
//子线程的运行函数wrapper
//说白了这个wrapper的作用只是为了进行一次系统调用缓存一下当前线程的id
void* threadFuncWrapper(void *args)
{
    Thread *t = static_cast<Thread *>(args);
    t->tid_ = currentThreadID();
    std::cout << "Sub thread " << t->tid_ << " created,ready to run." << std::endl;
    t->latch_.countdown();
    try
    {
        t->func_();
    }
    catch (...)
    {
        //TODO: log<<"thread "<<t->tid<<"run error."<<endl;
        throw;
    }
    return nullptr;
}
Thread::Thread(const threadFunc &tf,const std::string &name)
    : tid_(0), pthreadID_(0), running_(false), joined_(false), name_(name), latch_(1),
      func_(tf) {}

Thread::Thread(threadFunc &&tf,const std::string& name)
    : tid_(0), pthreadID_(0), running_(false), joined_(false),  name_(name),latch_(1),
      func_(std::move(tf)) {}

void Thread::run()
{
    if (!running_)
    {
        running_ = true;
        if (pthread_create(&pthreadID_, nullptr, threadFuncWrapper, this))
        {
            // TODO:log<<"thread create failed."<<endl;
            running_ = false;
        }
        else
        {
            latch_.wait();
            // TODO:log<<"thread "<<tid_<<"started."<<endl;
           // std::cout << "thread " << tid_ << "started." << std::endl;
        }
    }
}

void Thread::join()
{
    if (running_ && !joined_)
        pthread_join(pthreadID_,nullptr);
    joined_ = true;
    running_ = false;
}

void Thread::detach()
{
    if (running_ && !joined_)
    {
        //TODO:log<<"thread "<<tid_<<" detached.";
        pthread_detach(pthreadID_);
    }
}

Thread::~Thread()
{
    //detach();
    join();
}
} // namespace bases
