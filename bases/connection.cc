#include "connection.h"
#include "logger.h"
namespace net
{
Connection::Connection(event::EventLoop *loop, int fd, const sockaddr_in &local, const sockaddr_in &peer) : loop_(loop), event_(fd, loop), localAddress_(local), peerAddress_(peer)
{
    if(!loop)
        LOG_FATAL << "Event loop can't be null.";
}
Connection::~Connection() = default;
void Connection::handleRead()
{

}
void Connection::handleWrite()
{

}

void Connection::handleError()
{

}
} // namespace net