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
HttpServer::HttpServer(event::EventLoop *loop, const std::vector<int> &ports) : Server(loop, ports), requestTimeout_(10), aliveTimeout_(20)
{
    setReadCallback(std::bind(&net::HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}
HttpServer::~HttpServer() = default;
//收到http请求后最终调用到的回调
void HttpServer::onMessage(const ConnectionPtr &conn, bases::UserBuffer &buf)
{
    auto currentLoop = conn->getLoop();
    if (!currentLoop)
    {
        LOG_ERROR << "No event loop owns this connection,closed.";
        //服务端主动关闭连接
        conn->handleClose();
        return;
    }
    std::shared_ptr<HttpRequest> req = conn->getContext();
    //首先创建一个http request请求的上下文
    if (!req)
    {
        conn->setContext(std::shared_ptr<HttpRequest>(
            new HttpRequest(conn, requestTimeout_,
                            std::bind(&net::Connection::handleClose, conn.get()))));
        req = conn->getContext();
    }
    else //删除定时器
    {
        currentLoop->delTimer(req->getRequestTimer());
        //长连接复用请求时的初始状态是wait,到这里需要删除长连接的定时器
        if (req->getStatus() == WAIT)
        {
            currentLoop->delTimer(req->getAliveTimer());
            LOG_INFO << "Remove timer:" << req->getAliveTimer().format();
        }
    }

    //设置定时器
    req->setRequestTimer(requestTimeout_);
    //LOG_INFO << "Set timer,timeout at:" << req->getRequestTimer().format();
    currentLoop->addTimer(req->getRequestTimer());
    auto status = req->parse(buf);
    //解析出错
    if (status > DONE)
    {
        auto peer(conn->getPeerAddress());
        LOG_ERROR << "Parse http request error:" << req->status2String()
                  << " from " << peer.first << ":" << peer.second << ".";
        currentLoop->delTimer(req->getRequestTimer());
        //400:bad request
        if (conn->send(HttpResponse(req->getVersion(), HTTP_BAD_REQUEST)) == -1)
        {
            conn->handleClose();
            return;
        }
        conn->setState(DISCONNECTING);
        req.reset();
    }
    else if (status == DONE)
    {
        auto peer(conn->getPeerAddress());
        //LOG_INFO << "Parse http request from " << peer.first << ":" << peer.second << " done.";
        currentLoop->delTimer(req->getRequestTimer());
        //请求的资源路径
        auto requestPath(req->getRequestPath());
        //TODO:构造响应 200 400 404 408
        //DONE:完成
        if (conn->send(HttpResponse(req->getVersion(), requestPath)) == -1)
        {
            conn->handleClose();
            return;
        }
        if (req->getHeader("Connection") != "Keep-Alive" &&
            req->getHeader("Connection") != "keep-alive")
        {
            //NOTE:由于webbench、ab压测时都需要服务端主动关闭连接(默认短连接)，所以这里在解析完请求之后主动设置当前
            //连接状态为准备断开状态，以便写事件回调在写完响应到套接字内核缓冲区后能够及时关闭连接；
            conn->setState(DISCONNECTING);
            req.reset();
        }
        else //长连接
        {
            LOG_INFO << "Keep alive to:" << peer.first << ":" << peer.second;
            conn->resetBuffer();
            req->resetRequest();
            req->setAliveTimer(aliveTimeout_);
            LOG_INFO << "Set timer,timeout at:" << req->getAliveTimer().format();
            currentLoop->addTimer(req->getAliveTimer());
        }
    }
}
} // namespace net