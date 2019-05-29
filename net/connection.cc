#include <arpa/inet.h>
#include "connection.h"
#include "httpResponse.h"
#include "httpRequest.h"
#include "../event/eventLoop.h"
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
        {
            LOG_DEBUG << "Connection is being closed.";
            handleClose();
        }
        //对端关闭或半关闭了连接
        if (rc == 0)
            state_ = DISCONNECTING;
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
        if (state_ == DISCONNECTING && out_.empty())
        {
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
        /*
        if (context_.has_value())
        {
            HttpRequest *req = std::any_cast<HttpRequest>(&context_);
            loop_->delTimer(req->getRequestTimer());
            //loop_->delTimer(req->getAliveTimer());
        }
        else
        {
            LOG_INFO << "Request is null.";
        }
        */

        event_.remove();
        if (closeCallback_)
            closeCallback_(shared_from_this());
    }
}

} // namespace net