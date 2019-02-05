#include <arpa/inet.h>
#include "eventLoop.h"
#include "connection.h"
#include "httpResponse.h"
#include "httpRequest.h"
#include "logger.h"
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
    event_.setErrorCallback(std::bind(&Connection::handleError, this));
    event_.setCloseCallback(std::bind(&Connection::handleClose, this));
    //event_.enableAll();
}
Connection::~Connection() = default;
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
    if (state_ == CONNECTED)
    {
        int rc = in_.recv(event_.getFd());
        if (rc == -1)
            handleClose();
        //对端关闭或半关闭了连接
        if (rc == 0)
            state_ = DISCONNECTING;
        //LOG_INFO << "Received " << rc << " bytes.";
        if (rc && readCallback_)
            readCallback_(shared_from_this(), in_);
    }
}

void Connection::handleWrite()
{
    if (state_ != DISCONNECTED)
    {
        //LOG_INFO<<"Write event ready.";
        auto rc = out_.sendRemain(event_.getFd());
        //写操作出错，有可能是对端终止了连接
        if (rc == -1)
        {
            handleClose();
            return;
        }

        if (writeCallback_)
            writeCallback_(out_);
        if (state_ == DISCONNECTING && out_.isEmpty())
        {
            //LOG_INFO << "Disconnecting,close connection.";
            handleClose();
        }
    }
}
void Connection::handleError()
{
    if (state_ == CONNECTED)
    {
        if (errorCallback_)
            errorCallback_();
    }
}
//以下三个为发送响应的函数  并不是写事件回调
int Connection::send(const char *msg, int len)
{
    return out_.send(event_.getFd(), msg, len);
}
int Connection::send(const std::string &msg)
{
    return send(msg.c_str(), msg.size());
}
int Connection::send(const HttpResponse &response)
{
    return response.sendResponse(shared_from_this());
    //LOG_INFO<<"Response send completed.";
}
void Connection::eventInit()
{
    loop_->assertInOwnerThread();
    event_.tie(shared_from_this());
    event_.enableAll();
}
void Connection::handleClose()
{
    //loop_->assertInOwnerThread();
    //ref:2  map_+here
    //LOG_INFO << "Current reference count:" << shared_from_this().use_count();
    //防止已经close了连接
    //FIXME:防止定时器回调重复close
    //FIXED:加入了state的判断
    if (state_ != DISCONNECTED)
    {
        state_ = DISCONNECTED;
        event_.disableAll();
        //QUESTION:如果epoll把当前就绪的事件放入到rdlist后刚好进行remove操作，那么会如何？
        event_.remove();
        //FIXED:删除定时器防止后续重复close
        if (context_)
        {
            loop_->delTimer(context_->getRequestTimer());
            loop_->delTimer(context_->getAliveTimer());
        }
        if (closeCallback_)
            closeCallback_(event_.getFd());
    }
}

} // namespace net