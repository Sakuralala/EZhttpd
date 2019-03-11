#include <netinet/in.h>
#include <netinet/tcp.h>//tcpnodelay
#include <arpa/inet.h>
#include <sys/socket.h>
#include "acceptor.h"
#include "../bases/utils.h"
#include "../event/eventLoop.h"
#include "../log/logger.h"
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
    LOG_INFO << "Listen on file descriptor:" << listenFd_;
    //FIXME:port可能重复
    if (listenFd_ == -1)
        return false;
    bases::setNonBlocking(listenFd_);

    //NOTE:don't use map_[port]=event::Event(listenFd_,loop_)
    //since event::Event has no default ctor, however,
    //unordered_map::operator[] needs the mapped type has default ctor.
    std::unique_ptr<event::Event> ev(new event::Event(listenFd_, loop_));
    ev->setReadCallback(std::bind(&Acceptor::accept, this, listenFd_));
    ev->enableRead();
    map_.emplace(port, std::move(ev));
    return true;
}
//新连接到来时的回调
void Acceptor::accept(int listenFd)
{
    loop_->assertInOwnerThread();
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    //后面的标志位是为了避免两步操作破坏原子性
    while (true)
    {
        int acceptedFd = accept4(listenFd, (sockaddr *)&clientAddr, &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (acceptedFd == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                LOG_ERROR << "Accept on socket:" << listenFd << " error:" << strerror(errno);
            }
            return;
        }
        int val = 1;
        ::setsockopt(acceptedFd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
        char buf[INET_ADDRSTRLEN];
        /*
        if (!inet_ntop(AF_INET, &clientAddr.sin_addr, buf, INET_ADDRSTRLEN))
        {
            LOG_ERROR << "Invalid ip address.";
        }
        */
        //FIXME:此处显示的端口号和netstat/lsof命令显示的不一致
        //FIXED:addrLen需要传入确定的值
        LOG_DEBUG << "New connection:" << buf << ":" << ntohs(clientAddr.sin_port);
        if (onConnection_)
            onConnection_(acceptedFd, clientAddr);
    }
}
} // namespace net