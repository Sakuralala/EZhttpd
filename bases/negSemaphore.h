#ifndef _negsemaphore_h
#define _negsemaphore_h
#include "condition.h"

namespace bases
{
//负的信号量，意思就是负数或0时表示可用，正数时表示不可用；
class NegSemaphore : Nocopyable
{
  public:
    explicit NegSemaphore(const int cnt)
        : count_(cnt), mutex_(), cond_(mutex_) {}
    ~NegSemaphore() = default;
    void wait()
    {
        MutexGuard mg(mutex_);
        while (count_>0)
            cond_.wait();
        //为了能够多次使用这个countdownLatch 这样一改这其实就是一个信号量了。。
        count_++;
    }

    void countdown()
    {
        MutexGuard mg(mutex_);
        if (--count_ <= 0)
            cond_.notifyAll();
    }

  private:
    int count_;
    Mutex mutex_;
    Condition cond_;
};
} // namespace bases
#endif