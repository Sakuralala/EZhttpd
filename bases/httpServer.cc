#include <utility>
#include "httpServer.h"
#include "eventLoop.h"
#include "circularBuffer.h"
#include "timer.h"
#include "connection.h"
#include "httpRequest.h"
#include "httpResponse.h"
#include "logger.h"
namespace net
{
HttpServer::HttpServer(event::EventLoop *loop, const std::vector<int> &ports) : Server(loop, ports)
{
    requestTimeout_ = 15;
    aliveTimeout_ = 30;
    setReadCallback(std::bind(&net::HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}
//收到http请求后最终调用到的回调
void HttpServer::onMessage(const ConnectionPtr &conn, bases::UserBuffer &buf)
{
    auto currentLoop = conn->getLoop();
    if (!currentLoop)
    {
        LOG_ERROR << "No event loop owns this connection,closed.";
        conn->close();
    }
    std::shared_ptr<HttpRequest> req = conn->getContext();
    //首先创建一个http request请求的上下文
    if (!req)
    {
        conn->setContext(std::shared_ptr<HttpRequest>(new HttpRequest(conn, requestTimeout_, std::bind(&net::Connection::close, conn.get()))));
        req = conn->getContext();
        //为了防止对端没发完请求就关闭了连接而占用资源，需要设置定时器
        //currentLoop->addTimer()
    }
    else //删除定时器
    {
        currentLoop->delTimer(req->getTimer());
    }
    //设置定时器
    req->setTimer(requestTimeout_);
    currentLoop->addTimer(req->getTimer());
    auto status = req->parse(buf);
    //解析出错
    if (status > DONE)
    {
        auto peer(conn->getPeerAddress());
        LOG_ERROR << "Parse http request error:" << req->status2String()
                  << " from " << peer.first << ":" << peer.second << ".";
        //400:bad request
        conn->send(HttpResponse(req->getVersion(), HTTP_BAD_REQUEST));
        req.reset();
        conn->close();
    }
    else if (status == DONE)
    {
        auto peer(conn->getPeerAddress());
        LOG_INFO << "Parse http request from " << peer.first << ":" << peer.second << " done.";
        //请求的资源路径
        auto requestPath(req->getRequestPath());
        //TODO:构造响应 200 400 404 408
        conn->send(HttpResponse(req->getVersion(), HTTP_OK));
        if (req->getHeader("Connection") != "Keep-Alive" &&
            req->getHeader("Connection") != "keep-alive")
        {
            req.reset();
            conn->close();
        }
        else //长连接
        {
        }
    }
}
} // namespace net