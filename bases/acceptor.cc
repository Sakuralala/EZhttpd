#include <netinet/in.h>
#include <arpa/inet.h>
#include "acceptor.h"
#include "eventLoop.h"
#include "logger.h"
#include "utils.h"
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
    event::Event ev(listenFd_, loop_);
    ev.setReadCallback(std::bind(&Acceptor::accept, this, listenFd_));
    ev.enableRead();
    //TODO:move
    map_.emplace(port, std::move(ev));
    return true;
}
//新连接到来时的回调
void Acceptor::accept(int listenFd)
{
    loop_->assertInOwnerThread();
    struct sockaddr_in clientAddr;
    socklen_t addrLen = 0;
    //bzero();
    //后面的标志位是为了避免两步操作破坏原子性
    int acceptedFd = accept4(listenFd, (sockaddr *)&clientAddr, &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (acceptedFd == -1)
    {
        LOG_ERROR << "Accept failed in socket:" << listenFd << ".";
        return;
    }
    char buf[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &clientAddr.sin_addr, buf, INET_ADDRSTRLEN))
    {
        LOG_ERROR << "Invalid ip address.";
    }
    LOG_INFO << "Client:" << buf << " connected.";
    if (onConnection_)
        onConnection_(acceptedFd, clientAddr);
}
} // namespace net