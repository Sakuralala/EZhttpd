#include "thread.h"
#include "threadPool.h"
#include "../log/logger.h"

namespace bases
{
ThreadPool::ThreadPool(int qSize) : running_(false), eMode_(Immediately), maxTaskNum_(qSize), mutex_(), notEmpty_(mutex_), notFull_(mutex_)
{
}
ThreadPool::~ThreadPool()
{
    stop();
}
void ThreadPool::run(int threadNum)
{
    if (threadNum > 0 && !running_)
    {
        LOG_DEBUG << "Thread pool started.";
        //std::cout << "Thread pool started." << std::endl;
        running_ = true;
        threads_.reserve(threadNum);
        while (threadNum--)
        {
            threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadFunction, this), "Worker"));
            threads_.back()->run();
        }
    }
}

void ThreadPool::stop(ExitMode em)
{
    if (running_)
    {
        {
            MutexGuard mg(mutex_);
            eMode_ = em;
            running_ = false;
            notEmpty_.notifyAll();
        }
        for (auto &thread : threads_)
        {
            thread->join();
            //std::cout << "thread " << thread->getTid() << " destroyed." << std::endl;
        }
        //std::cout << "Thread pool destroyed, current task queue size:" << taskQueue_.size() << std::endl;
        LOG_DEBUG << "Thread pool destroyed, current task queue size:" << taskQueue_.size();
    }
}
ThreadPool::Task ThreadPool::get()
{
    MutexGuard mg(mutex_);
    //std::cout << "Sub thread:" << currentThreadID() << ",current task queue size:" << taskQueue_.size() << std::endl;
    //running_是为了让子线程在停止时能够顺利地退出
    while (running_ && taskQueue_.empty())
    {
        //std::cout << "Sub thread:" << currentThreadID() << ",current task queue is empty,go to sleep." << std::endl;
        notEmpty_.wait();
    }
    //move ctor
    Task task;
    if (running_ || (eMode_ == Graceful && !taskQueue_.empty()))
    {
        if (!running_)
        {
            //std::cout << "Sub thread:" << currentThreadID() << ",do some final works,current task queue size:" << taskQueue_.size() << std::endl;
            LOG_DEBUG << "Sub thread:" << currentThreadID() << ", do some final works, current task size:" << taskQueue_.size();
        }
        task = std::move(taskQueue_.front());
        taskQueue_.pop_front();
        notFull_.notify();
    }
    //move ctor
    return task;
}

void ThreadPool::put(Task task)
{
    MutexGuard mg(mutex_);
    while (taskQueue_.size() == maxTaskNum_)
        notFull_.wait();
    taskQueue_.push_back(std::move(task));
    notEmpty_.notify();
}

void ThreadPool::threadFunction()
{
    try
    {
        //TODO:这里存在race condition 因为没在判断task queue是否为空的时候获取锁
        //但是这是最简单的work around的方法了 因为这个判断的实时性对最终的结果没
        //影响 就算判断的时候或者判断完之后task queue中的任务被拿完了 那么子线程
        //也只是会多进一次get及多进行一次while的判断而已
        //另一种方法是不要get函数  直接把内容写到while中，while的条件改为true 跳出在
        //循环中进行
        //DONE:使用了goto语句
        while (running_)
        {
            Task task(get());
            if (task)
                task();
        }
    //NOTE:使用goto??
    Graceful:
    {
        MutexGuard mg(mutex_);
        if ((eMode_ == Graceful && !taskQueue_.empty()))
        {
            Task task(get());
            if (task)
                task();
            goto Graceful;
        }
    }
    }
    catch (...)
    {
        LOG_FATAL << "threadpool error occured.";
        throw;
    }
}
} // namespace bases