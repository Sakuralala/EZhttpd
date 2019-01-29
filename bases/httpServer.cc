#include "httpServer.h"
#include "eventLoop.h"
#include "circularBuffer.h"
#include "connection.h"
#include "httpRequest.h"
namespace net
{
HttpServer::HttpServer(event::EventLoop *loop, std::vector<int> &ports) : Server(loop, ports)
{
    setReadCallback(std::bind(&net::HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}
void HttpServer::onMessage(ConnectionPtr &conn, bases::UserBuffer &buf)
{
    if (!conn->getContext())
        conn->setContext(new HttpRequest());
}
} // namespace net