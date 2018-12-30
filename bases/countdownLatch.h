#ifndef _countdownLatch_h
#define _countdownLatch_h
#include "condition.h"

namespace bases
{
class CountdownLatch : Nocopyable
{
  public:
    explicit CountdownLatch(const int cnt)
        : count_(cnt), mutex_(), cond_(mutex_) {}
    ~CountdownLatch() = default;
    void wait()
    {
        MutexGuard mg(mutex_);
        while (count_)
            cond_.wait();
    }

    void countdown()
    {
        MutexGuard mg(mutex_);
        if (--count_ == 0)
            cond_.notifyAll();
    }

  private:
    int count_;
    Mutex mutex_;
    Condition cond_;
};
} // namespace bases
#endif