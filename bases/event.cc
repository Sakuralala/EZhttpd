#include <sys/epoll.h>
#include "event.h"
#include "eventLoop.h"
namespace event
{
//EPOLLERR、EPOLLHUP这两种类型的事件,epoll强制置位，不需要调用者手动置位
const int Event::WriteEvent= EPOLLOUT;
const int Event::ReadEvent = EPOLLIN | EPOLLPRI;
//需要监控的事件对应的文件描述符，感兴趣的事件类型
Event::Event(int fd, EventLoop *loop) : fd_(fd), operation_(-1),readyType_(0), loop_(loop)
{
    //仅工作在ET模式下
    //interestedType_ |= EPOLLET;
}
Event::~Event()
{
}
void Event::update()
{
    //race condition   如果某个其他线程调用了enable*,然后这里需要修改operation_
    //而owner线程正好在getOperation(或者也在update),则会有race condition
    //所以需要保证update由owner线程调用
    loop_->assertInOwnerThread();
    if (operation_ == -1)
        operation_ = EPOLL_CTL_ADD;
    else
        operation_ = EPOLL_CTL_MOD;
    loop_->updateEvent(this);
}

void Event::remove()
{
    //loop_->assertInOwnerThread();
    operation_ = EPOLL_CTL_DEL;
    loop_->updateEvent(this);
}

//处理相应的事件
void Event::handleEvent()
{
    if(readyType_&(ReadEvent|EPOLLHUP|EPOLLRDHUP))
    {
        if(readCallback_)
            readCallback_();
    }
    else if(readyType_&WriteEvent)
    {
        if(writeCallback_)
            writeCallback_();
    }
    else if(readyType_&EPOLLERR)
    {
        if(errorCallback_)
            errorCallback_();
    }
}
} // namespace event