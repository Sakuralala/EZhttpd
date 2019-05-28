#include <sys/epoll.h>
#include <unistd.h>
#include "event.h"
#include "eventLoop.h"
#include "../log/logger.h"
namespace event
{
//EPOLLERR、EPOLLHUP这两种类型的事件,epoll强制置位，不需要调用者手动置位
const int Event::WriteEvent = EPOLLOUT;
const int Event::ReadEvent = EPOLLIN | EPOLLPRI;
//需要监控的事件对应的文件描述符，感兴趣的事件类型
Event::Event(int fd, EventLoop *loop) : monitored(false), fd_(fd), operation_(-1), readyType_(0),
                                        interestedType_(0), loop_(loop), tied_(false)
{
    if (!loop_)
    {
        LOG_FATAL << "No event loop for current event.";
    }
    //仅工作在ET模式下
    interestedType_ |= EPOLLET;
}
Event::Event(Event &&ev)
{
    fd_ = ev.fd_;
    ev.fd_ = -1;
    operation_ = ev.operation_;
    ev.operation_ = -1;
    readyType_ = ev.readyType_;
    ev.readyType_ = -1;
    interestedType_ = ev.interestedType_;
    ev.interestedType_ = -1;
    loop_ = ev.loop_;
    ev.loop_ = nullptr;
    readCallback_ = ev.readCallback_;
    writeCallback_ = ev.writeCallback_;
    errorCallback_ = ev.errorCallback_;
}
Event::~Event()
{
    if (monitored)
        remove();
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
void Event::tie(const std::shared_ptr<void> &owner)
{
    loop_->assertInOwnerThread();
    tied_ = true;
    owner_ = owner;
}
//处理相应的事件
void Event::handleEvent()
{
    //网络事件强制需要tie
    if (tied_)
    {
        std::shared_ptr<void> guard(owner_.lock());
        if (guard)
        {
            _handleEvent();
        }
    }
    else //监听、wakeup事件是不需要tie的，因为它们不对应一个Connection对象
    {
        _handleEvent();
    }
}
void Event::_handleEvent()
{
    //FIXME:EPOLLHUP事件的处理 从man epoll_ctl可知，这个事件仅仅表示对端关闭了连接
    //FIXED:我们还需要先把对端在关闭连接之前发送的数据读完才行 这个时候才能关闭连接
    //后续:EPOLLHUP一般表示本端出错？比如说对端非正常断开，服务端写会触发RST分节
    //然后就会导致EPOLLHUP被设置
    //这个好像也不用处理，道理和EPOLLERR一样
    /*
    if ((readyType_ & EPOLLHUP) && !(readyType_ & EPOLLIN))
    {
        if (closeCallback_)
            closeCallback_();
    }
    */
    /*
    if (readyType_ & EPOLLERR)
    {
        if (errorCallback_)
            errorCallback_();
    }
    */
    //由于某些原因出现了这个的话，让读写回调去处理即可
    if (readyType_ & (EPOLLERR | EPOLLHUP))
    {
        readyType_ = EPOLLIN | EPOLLOUT;
    }
    //错误事件的话,由于在读写事件的回调中是一直读或写的，那么只要在这期间出错了就会关闭连接
    //故其实不需要检测错误事件,如果是在读写之后的某个时间点发生了错误，那么服务端是检测不出来的
    //因为EPOLLERR只有在服务端采取了动作之后才能检测地出，这就是需要设置服务端超时回调的原因
    //因为服务端不可能没事就读一个套接字；
    if (readyType_ & (ReadEvent | EPOLLRDHUP))
    {
        if (readCallback_)
            readCallback_();
    }
    if (readyType_ & WriteEvent)
    {
        //LOG_INFO << " Write event ready in socket:" << fd_;
        if (writeCallback_)
            writeCallback_();
    }
}
} // namespace event