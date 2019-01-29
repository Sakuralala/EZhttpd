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
//收到http请求后最终调用到的回调 
void HttpServer::onMessage(const ConnectionPtr &conn, bases::UserBuffer &buf)
{
    //首先创建一个http request请求的上下文
    if (!conn->getContext())
        conn->setContext(std::shared_ptr<HttpRequest>(new HttpRequest));

    std::shared_ptr<HttpRequest> req = conn->getContext();
    req->parse(buf);
}
} // namespace net