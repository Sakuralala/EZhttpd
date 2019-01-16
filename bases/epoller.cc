#include <sys/epoll.h>
#include <string.h>//memset
#include "epoller.h"
#include "event.h"
#include "logger.h"
#include "thread.h"
namespace event
{
const int InitialEventsNumber = 2048;
Epoller::Epoller() : epollFd_(::epoll_create1(EPOLL_CLOEXEC)), events_(InitialEventsNumber)
{
}
Epoller::~Epoller() = default;
//更新当前连接上感兴趣的事件
bool Epoller::updateEvent(Event *ev)
{
    int fd = ev->getFd();
    int op = ev->getOperation();
    if(op==-1)
    {
        op = EPOLL_CTL_ADD;
        ev->setOperation(EPOLL_CTL_ADD);
    }
    epoll_event ee;
    memset(&ee, 0, sizeof(epoll_event));
    ee.events = ev->getInterestedType();
    ee.data.ptr = static_cast<void *>(ev);

    if (op == EPOLL_CTL_ADD)
    {
        ev->setOperation(EPOLL_CTL_MOD);
        if (::epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ee))
        {
            LOG_ERROR<<"epoll add error occured in thread:"<<bases::currentThreadID();
            return false;
        }
    }
    else if (op == EPOLL_CTL_MOD)
    {
        if (::epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ee))
        {
            LOG_ERROR<<"epoll mod error occured in thread:"<<bases::currentThreadID();
            return false;
        }
    }
    else
    {
        if (::epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ee))
        {
            LOG_ERROR<<"epoll del error occured in thread:"<<bases::currentThreadID();
            return false;
        }
    }
    return true;
}
int Epoller::poll(int ms, EventList &el)
{
    int n = epoll_wait(epollFd_, &*events_.begin(), events_.size(), ms);
    for (int i = 0; i < n; ++i)
    {
        el.push_back(static_cast<Event *>(events_[i].data.ptr));
        //具体的就绪事件类型
        el[i]->setReadyType(events_[i].events);
    }
    if (n == events_.size())
    {
        events_.resize(2 * events_.size());
        LOG_INFO<<"Epoll event list resized,current size:"<<events_.size();
    }
    return n;
}
} // namespace event