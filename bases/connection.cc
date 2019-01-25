#include "connection.h"
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
}
Connection::~Connection() = default;
void Connection::handleRead()
{
    in_.recv(event_.getFd());
    if (readCallback_)
        readCallback_(this->shared_from_this(), in_);
}
void Connection::handleWrite()
{
    out_.sendRemain(event_.getFd());
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
} // namespace net