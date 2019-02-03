#include <utility>
#include <sys/time.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <stdio.h>  //for snprintf
#include <string.h> //for memset
#include "timer.h"
#include "logger.h"
namespace event
{
//周几
static const char *DayofWeek[7] = {"Monday", "Tuesday", "Wednsday", "Thursday", "Friday", "Saturday", "Sunday"};
//timernode
Timer::Timer(uint64_t secs)
{
    setTime(secs);
}
void Timer::setTime(uint64_t secs)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    milliSeconds_ = (secs + tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}
Timer::Timer(uint64_t secs, TimeoutCallback cb) : Timer(secs)
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
    //LOG_INFO << "Timeout, executing callback....";
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
    //LOG_INFO << "Current time:" << cur.format();
    for (auto ite = timerSet_.begin(); ite != last; ++ite)
    {
        //LOG_INFO << "Timer:" << ite->second.format() << " timeout.";
        ite->second.timeout();
    }

    timerSet_.erase(timerSet_.begin(), last);
}

/*
void TimerSet::add(uint64_t secs, Callback cb)
{
    timerSet_.insert(Timer(secs, std::move(cb)));
}
*/
void TimerSet::add(const Timer &t)
{
    if (!t.getCallback())
    {
        LOG_ERROR << "No timeout callback set!";
        return;
    }
    //LOG_INFO << "Add timer:" << t.getTime() << " to timerset.";
    timerSet_.emplace(TimerKey(&t), t);
}

void TimerSet::del(const Timer &t)
{
    timerSet_.erase(TimerKey(&t));
}
//查看最近超时的定时器的超时时间
uint64_t TimerSet::getNext() const
{
    return timerSet_.begin()->second - Timer::now();
}
} // namespace event