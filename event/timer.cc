#include <utility>
#include <sys/time.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <stdio.h>  //for snprintf
#include <string.h> //for memset
#include "timer.h"
#include "../log/logger.h"
namespace event
{
//周几 外国人是以周日为一周的起点的。。。
static const char *DayofWeek[7] = {"Sunday","Monday", "Tuesday", "Wednsday", "Thursday", "Friday", "Saturday"};
//timernode
Timer::Timer(uint64_t secs, uint64_t repeats) : repeatTimes_(repeats), seconds_(secs)
{
    setTimer(secs);
}
void Timer::setTimer(uint64_t secs)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    milliSeconds_ = (secs + tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}
void Timer::resetTimer()
{
    LOG_DEBUG << "Reset timer to " << seconds_ << " second(s) later.";
    setTimer(seconds_);
}
Timer::Timer(uint64_t secs, TimeoutCallback cb, uint64_t repeats) : Timer(secs, repeats)
{
    //for this reason:
    //"an initializer for a delegating constructor must appear alone",I put it here.
    timeoutCallback_ = std::move(cb);
}
//获取当前时间
Timer Timer::now()
{
    return Timer(0);
}
void Timer::timeout()
{
    LOG_DEBUG << "Timeout, executing callback....";
    if (timeoutCallback_)
        timeoutCallback_();
}
std::string Timer::format() const
{
    return format(milliSeconds_);
}
std::string Timer::format(uint64_t millisecs)
{
    char buf[64];
    memset(buf, 0, sizeof(char) * 64);
    struct tm formatedTime;
    time_t seconds = static_cast<time_t>(millisecs / 1000);
    gmtime_r(&seconds, &formatedTime);
    snprintf(buf, sizeof(buf), "%4d:%02d:%02d %02d:%02d:%02d,%s,UTC ",
             formatedTime.tm_year + 1900, formatedTime.tm_mon + 1, formatedTime.tm_mday,
             formatedTime.tm_hour, formatedTime.tm_min, formatedTime.tm_sec, DayofWeek[formatedTime.tm_wday]);
    return buf;
}

//timerset
void TimerSet::getExpired()
{
    auto cur = Timer::now();
    TimerKey tk(&cur);
    //找到第一个大于cur的定时器
    auto last = timerSet_.upper_bound(tk);
    if (last == timerSet_.begin())
        return;
    //需要更新的定时器
    std::vector<TimerPtr> needUpdate;
    for (auto ite = timerSet_.begin(); ite != last; ++ite)
    {
        ite->second->timeout();
        if (ite->second->getRepeatTimes() != 1)
        {
            ite->second->decreaseRepeatTimes();
            ite->second->resetTimer();
            needUpdate.emplace_back(std::move(ite->second));
        }
    }
    timerSet_.erase(timerSet_.begin(),last);
    for(auto &elem:needUpdate)
    {
        timerSet_.emplace(TimerKey(elem.get()),std::move(elem));
    }
}

TimerKey TimerSet::add(uint64_t secs, Callback cb, uint64_t interval)
{
    std::unique_ptr<Timer> tp(new Timer(secs, cb, interval));
    TimerKey tk(tp.get());
    timerSet_.emplace(tk, std::move(tp));
    return tk;
}
/*
void TimerSet::add(const Timer &t)
{
}
*/
void TimerSet::del(const TimerKey &tk)
{
    if (!timerSet_.erase(tk))
        LOG_DEBUG << "No such timer.";
}
//查看最近超时的定时器的超时时间
uint64_t TimerSet::getNext() const
{
    return *(timerSet_.begin()->second.get()) - Timer::now();
}
} // namespace event