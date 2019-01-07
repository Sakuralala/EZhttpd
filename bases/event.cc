#include <sys/epoll.h>
#include "event.h"
#include "eventLoop.h"
namespace event
{
//EPOLLERR、EPOLLHUP这两种类型的事件,epoll强制置位，不需要调用者手动置位
const int Event::WriteEvent= EPOLLOUT;
const int Event::ReadEvent = EPOLLIN | EPOLLPRI;
//需要监控的事件对应的文件描述符，感兴趣的事件类型
Event::Event(int fd, EventLoop *loop) : fd_(fd),mutex_(), operation_(-1),readyType_(0), loop_(loop)
{
    if(!loop_)
    {
        //TODO:log_error<<"No event loop for current event.";
    }
    //仅工作在ET模式下
    interestedType_ |= EPOLLET;
}
Event::~Event()
{
}
//通用的调用次序:Event::enable/disable*->Event::update->EventLoop::updateEvent->Epoller::updateEvent
void Event::update()
{
    /*
    //race condition   如果某个其他线程调用了enable*,然后这里需要修改operation_
    //而owner线程正好在getOperation(或者也在update),则会有race condition
    //所以需要保证update由owner线程调用
    //如果这里采用改成加锁的话，似乎也可行；另外，如果enable/disable*都需要通过std::bind()来
    //进行任务的添加，感觉很奇怪
    //另一种做法，使用mutex，这样就可以跨线程调用enable/disable*
    loop_->assertInOwnerThread();

    if (operation_ == -1)
        operation_ = EPOLL_CTL_ADD;
    else
        operation_ = EPOLL_CTL_MOD;
    */
   //另一种方式 由epoller来进行operation的更新
    loop_->updateEvent(this);
}

void Event::remove()
{
    loop_->assertInOwnerThread();
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