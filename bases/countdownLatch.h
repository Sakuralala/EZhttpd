#ifndef _countdownlatch_h
#define _countdownlatch_h
#include "condition.h"

namespace bases
{
class CountdownLatch : Nocopyable
{
  public:
    explicit CountdownLatch(const int cnt)
        : count_(cnt), initialCount_(cnt), mutex_(), cond_(mutex_) {}
    ~CountdownLatch() = default;
    void wait()
    {
        MutexGuard mg(mutex_);
        while (count_ > 0)
            cond_.wait();
    }

    void countdown()
    {
        MutexGuard mg(mutex_);
        if (--count_==0)
            cond_.notifyAll();
    }
    //重启倒计时
    void reset()
    {
        MutexGuard mg(mutex_);
        count_ = initialCount_;
    }

  private:
    int count_;
    const int initialCount_;
    Mutex mutex_;
    Condition cond_;
};
} // namespace bases
#endif