#include "acceptor.h"
#include "bases/eventLoop.h"
#include "bases/logger.h"
#include "bases/util.h"
namespace net
{
Acceptor::Acceptor(event::EventLoop *loop)
{
    if (!loop)
        LOG_FATAL << "Acceptor loop can't be null!";
    loop_ = loop;
}

Acceptor::~Acceptor() = default;

//监听某个端口
bool Acceptor::listen(int port)
{
    loop_->assertInOwnerThread();
    int listenFd_ = bases::listen(port);
    //FIXME:port可能重复
    if (listenFd_ == -1)
        return false;
    bases::setNonBlocking(listenFd_);
    //NOTE:don't use map_[port]=event::Event(listenFd_,loop_) since event::Event has no default ctor, however, unordered_map::operator[] needs the mapped type has default ctor.
    map_.emplace(port, event::Event(listenFd_, loop_));
    map_[port].setReadCallback(std::bind(&Acceptor::accept, this, listenFd_));
    map_[port].enableRead();
    return true;
}
//新连接到来时的回调
void Acceptor::accept(int listenFd)
{
    loop_->assertInOwnerThread();
    struct sockaddr_in clientAddr;
    //bzero();
    //后面的标志位是为了避免两步操作破坏原子性
    int acceptedFd = accept4(listenFd, (sockaddr *)&clientAddr, sizeof clientAddr, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (acceptedFd == -1)
    {
        LOG_ERROR << "Accept failed in socket:" << listenFd << ".";
        return;
    }
    if (onConnection_)
        onConnection_(acceptedFd, clientAddr);
}
} // namespace net