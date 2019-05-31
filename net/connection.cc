#include <arpa/inet.h>
#include "connection.h"
#include "httpResponse.h"
#include "httpRequest.h"
#include "../event/eventLoop.h"
#include "../event/event.h"
#include "../log/logger.h"
namespace net
{
Connection::Connection(event::EventLoop *loop, int fd,
                       const sockaddr_in &local, const sockaddr_in &peer) : state_(CONNECTED),
                                                                            loop_(loop), event_(fd, loop), localAddress_(local), peerAddress_(peer)
{
    if (!loop)
        LOG_FATAL << "Event loop can't be null.";
    event_.setReadCallback(std::bind(&Connection::handleRead, this));
    event_.setWriteCallback(std::bind(&Connection::handleWrite, this));
    //event_.setErrorCallback(std::bind(&Connection::handleError, this));
    //event_.setCloseCallback(std::bind(&Connection::handleClose, this));
    //event_.enableAll();
}
Connection::~Connection()
{
    //NOTE:fd的生命周期由Connection控制
    LOG_INFO << "Close fd: " << getFd();
    ::close(getFd());
}
std::pair<std::string, uint16_t> Connection::getLocalAddress() const
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &localAddress_.sin_addr, buf, INET_ADDRSTRLEN);
    return std::pair<std::string, uint16_t>(buf, ntohs(peerAddress_.sin_port));
}
std::pair<std::string, uint16_t> Connection::getPeerAddress() const
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peerAddress_.sin_addr, buf, INET_ADDRSTRLEN);
    //都是网络字节序的
    return std::pair<std::string, uint16_t>(buf, ntohs(peerAddress_.sin_port));
}
void Connection::handleRead()
{
    if (state_ != DISCONNECTED)
    {
        int rc = in_.recv(event_.getFd());
        if (rc == -1)
        {
            LOG_DEBUG << "Connection is being closed.";
            handleClose();
        }
        //对端关闭或半关闭了连接
        //FIXME:写的时候只考虑了server端的情况，没考虑client端的情况
        //对于server而言，rc为0表示client端
        if (rc == 0)
        {
            LOG_DEBUG << "Peer close or shutdown the connection.";
            //说明当前已经关闭了写半端，那么直接关闭整个连接
            if (state_ == DISCONNECTING)
            {
                LOG_DEBUG << "Close whole connection.";
                if (readCallback_)
                    readCallback_(shared_from_this(), in_);
                handleClose();
                return;
            }
            else if (state_ == CONNECTED) //当前本端还未断开连接，但是对端断开或半断开了
            {
                state_ = DISCONNECTING;
            }
        }
        LOG_DEBUG << "Get " << rc << " bytes data this time.";
        if (rc && readCallback_)
            readCallback_(shared_from_this(), in_);
    }
}

void Connection::handleWrite()
{
    if (state_ != DISCONNECTED)
    {
        auto rc = out_.sendRemain(event_.getFd());
        //写操作出错，有可能是对端终止了连接
        if (rc == -1)
        {
            handleClose();
            return;
        }

        if (writeCallback_)
            writeCallback_(out_);
        //FIXME:此处只考虑了server端，即server设置了状态为DISCONNECTING，并且需要发送的信息已经发完了, 那么就关闭连接，但是这对client端不适用,因为client端发完一般是关闭写半端的，等到收完完整的回复后由server端发起后两次挥手过程从而关闭连接
        if (state_ == DISCONNECTING && out_.empty() && event_.isWriting())
        {
            LOG_DEBUG << "Close the connection.";
            //FIXME:改为shutdownWrite()?但是对于server端来说，发完数据是可以直接close的
            //FIXED:将fd的管理交给Connection,在其析构的同时进行close,这样可以保证资源的释放 即使client端先关闭了写半端，这边server也在发完数据后关闭了写半端，只要两者中的一个先主动断开连接了导致connection析构，就能够保证正确的四次挥手？ server端也可以通过设置定时器来防止恶意不主动关闭连接的client
            //handleClose();
            shutdownWrite();
        }
    }
}
/*
void Connection::handleError()
{
    if (state_ == CONNECTED)
    {
        if (errorCallback_)
            errorCallback_();
    }
}
*/
//以下三个为发送响应的函数  并不是写事件回调
//FIXME:send是主动调用的，为避免race condition需要保证在io线程进行调用
//FIXED:两种解决办法，1 保证在owner线程中进行调用;2 非io线程时sendInLoop
//FIXME:send的错误处理; 如果在send内部处理错误并调用close callback, 那么此时有可能还在读事件的
//某层callback中，这会造成后续代码的继续执行;
//TODO:暂时的解决方法就是调用send的地方需要判断返回值;
int Connection::send(const char *msg, int len)
{
    loop_->assertInOwnerThread();
    return out_.send(event_.getFd(), msg, len);
}
int Connection::send(const std::string &msg)
{
    return send(msg.c_str(), msg.size());
}
int Connection::send(const HttpResponse &response)
{
    return response.sendResponse(shared_from_this());
}
void Connection::eventInit()
{
    loop_->assertInOwnerThread();
    event_.tie(shared_from_this());
    event_.enableAll();
}
void Connection::handleClose()
{
    //此callback被主动调用
    loop_->assertInOwnerThread();
    //ref:2  map_+here
    LOG_DEBUG << "Current reference count:" << shared_from_this().use_count();
    //防止已经close了连接
    //FIXME:防止定时器回调重复close
    //FIXED:加入了state的判断
    if (state_ != DISCONNECTED)
    {
        state_ = DISCONNECTED;
        //event_.disableAll();
        //QUESTION:考虑这么一种情况，当前请求超时准备删除，但此时从这个请求的另一端来了消息，那么epoll_wait下一次还会存在这个就绪事件吗?
        //ANSWER:看了下源码，会从rdlist中删除之；
        event_.remove();
        if (closeCallback_)
            closeCallback_(shared_from_this());
    }
}

void Connection::shutdownWrite()
{
    //此callback被主动调用
    loop_->assertInOwnerThread();
    setState(DISCONNECTING);
    if (outEmpty())
    {
        event_.disableWrite();
        LOG_DEBUG << "Shutdown write part.";
        //LOG_INFO << "Current intersted event type: "<<event_.getInterestedType();
        ::shutdown(getFd(), SHUT_WR);
    }
}

} // namespace net