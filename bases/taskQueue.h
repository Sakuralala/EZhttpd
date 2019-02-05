#ifndef _taskqueue_h
#define _taskqueue_h

#include <functional>
#include <deque>
#include <utility>
#include "nocopyable.h"
#include "condition.h"
/*
 * 实现了一个任务队列的类，此处并未进行模板化，因为通常task都是void(xxx)类型的，
 * 即最多参数有差别，那么可以使用std::bind来屏蔽掉这些差别使之兼容；
 * 注意此taskqueue不能作为threadpool的直接成员,因为假设我现在要退出服务端程序,然后调用了
 * 线程池的停止运行函数，那么在其内部主线程需要等待子线程join，但是如果当前taskqueue中没有task,
 * 最先拿到锁的子线程就一直阻塞在while循环那等待，而其他没拿到锁的线程则一直无法获取锁，
 * 而主线程又一直在等待子线程join，造成了死锁。  
 * 不过说实话服务端程序停止的时候，是不是正常终止好像也没啥所谓。直接kill好像也不是不行。 
 */
namespace bases
{
class TaskQueue : Nocopyable
{
  public:
    typedef std::function<void()> Task;
    explicit TaskQueue(int num) : maxTaskNum_(num), mutex_(), notEmpty_(mutex_), notFull_(mutex_)
    {
    }
    ~TaskQueue() = default;
    //其实就是生产者消费者模型  
    void put(const Task &task)
    {
        MutexGuard mg(mutex_);
        while(taskQueue_.size()==maxTaskNum_)
            notFull_.wait();
        taskQueue_.push_back(std::move(task));
        notEmpty_.notify();
    }

    Task take()
    {
        MutexGuard mg(mutex_);
        while(taskQueue_.empty())
            notEmpty_.wait();
        Task ret(std::move(taskQueue_.front()));
        taskQueue_.pop_front();
        notFull_.notify();
        return ret;
    }

  private:
    int maxTaskNum_;
    Mutex mutex_;
    Condition notEmpty_;
    Condition notFull_;
    std::deque<Task> taskQueue_;
};
} // namespace bases
#endif // !1