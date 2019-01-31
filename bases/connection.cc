#include <arpa/inet.h>
#include "connection.h"
#include "httpResponse.h"
#include "logger.h"
namespace net
{
Connection::Connection(event::EventLoop *loop, int fd, const sockaddr_in &local, const sockaddr_in &peer) : loop_(loop), event_(fd, loop), localAddress_(local), peerAddress_(peer)
{
    if (!loop)
        LOG_FATAL << "Event loop can't be null.";
    event_.setReadCallback(std::bind(&Connection::handleRead, this));
    event_.setWriteCallback(std::bind(&Connection::handleWrite, this));
    event_.setErrorCallback(std::bind(&Connection::handleError, this));
    event_.enableAll();
}
Connection::~Connection() = default;
std::pair<std::string, int> Connection::getLocalAddress() const
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &localAddress_.sin_addr, buf, INET_ADDRSTRLEN);
    return std::pair<std::string, int>(buf, ntohs(peerAddress_.sin_port));
}
std::pair<std::string, int> Connection::getPeerAddress() const
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peerAddress_.sin_addr, buf, INET_ADDRSTRLEN);
    return std::pair<std::string, int>(buf, ntohs(peerAddress_.sin_port));
}
void Connection::handleRead()
{
    in_.recv(event_.getFd());
    if (readCallback_)
    {
        auto connPtr(this->shared_from_this());
        readCallback_(connPtr, in_);
    }
}

void Connection::handleWrite()
{
    auto rc = out_.sendRemain(event_.getFd());
    //写操作出错，有可能是对端终止了连接
    if (rc == -1)
        close();

    if (writeCallback_)
        writeCallback_(out_);
}
void Connection::handleError()
{
    if (errorCallback_)
        errorCallback_();
}
void Connection::send(const char *msg, int len)
{
    out_.send(event_.getFd(), msg, len);
}
void Connection::send(const std::string &msg)
{
    send(msg.c_str(), msg.size());
}
void Connection::send(const HttpResponse &response)
{
    ConnectionPtr conn(this->shared_from_this());
    response.sendResponse(conn);
}
//TODO:
void Connection::close()
{
}
} // namespace net