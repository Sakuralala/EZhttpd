#include <utility>
#include "httpServer.h"
#include "connection.h"
#include "httpRequest.h"
#include "httpResponse.h"
#include "../event/eventLoop.h"
//#include "../bases/circularBuffer.h"
#include "../bases/newCircularBuffer.h"
#include "../event/timer.h"
#include "../log/logger.h"
namespace net
{
HttpServer::HttpServer(event::EventLoop *loop, const std::vector<int> &ports) : Server(loop, ports), requestTimeout_(10), aliveTimeout_(20)
{
    setReadCallback(std::bind(&net::HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    setCloseCallback(std::bind(&net::HttpServer::onClose, this, std::placeholders::_1));
}
HttpServer::~HttpServer() = default;
//http请求结束时的回调,注意最后需要调用server的delConnection
void HttpServer::onClose(const ConnectionPtr &conn)
{
    //FIXME:如果把参数改为fd，再通过映射表connected_查找得到conn再进行下面的操作就报
    //段错误? gdb显示has_value这个调用报错？？
    //我猜可能的原因是映射表connected_的多线程读写造成的race condition:可能由于换表？
    /*
    ConnectionPtr connCopy = connected_[conn->getFd()];
    LOG_INFO << "Current reference count:" << connCopy.use_count();
    if (connCopy.get() == conn.get())
        LOG_INFO << "Equal.";
    */
    if (conn->getContext()->has_value())
    {
        HttpRequest *req = std::any_cast<HttpRequest>(conn->getContext());
        conn->getLoop()->delTimer(req->getRequestTimerKey());
        conn->getLoop()->delTimer(req->getAliveTimerKey());
    }
    else
    {
        LOG_INFO << "Request is null.";
    }
    LOG_DEBUG << "Connection fd:" << conn->getFd() << " deleted.";
    delConnection(conn->getFd());
}
//收到http请求后最终调用到的回调
void HttpServer::onMessage(const ConnectionPtr &conn, bases::UserBuffer &buf)
{
    LOG_DEBUG << "New request accepted.";
    auto currentLoop = conn->getLoop();
    if (!currentLoop)
    {
        LOG_ERROR << "No event loop owns this connection,closed.";
        //服务端主动关闭连接
        conn->handleClose();
        return;
    }
    HttpRequest *req = std::any_cast<HttpRequest>(conn->getContext());
    //首先创建一个http request请求的上下文
    if (!req)
    {
        conn->setContext(HttpRequest(conn));
        req = std::any_cast<HttpRequest>(conn->getContext());
    }
    else //删除定时器
    {
        currentLoop->delTimer(req->getRequestTimerKey());
        //长连接复用请求时的初始状态是wait,到这里需要删除长连接的定时器
        if (req->getStatus() == WAIT)
        {
            currentLoop->delTimer(req->getAliveTimerKey());
            LOG_DEBUG << "Remove timer:" << req->getAliveTimerKey().format();
        }
    }

    //设置定时器
    req->setRequestTimerKey(currentLoop->addTimer(requestTimeout_, std::bind(&Connection::handleClose, conn.get())));
    LOG_DEBUG << "Set timer,timeout at:" << req->getRequestTimerKey().format();
    auto status = req->parse(buf);
    //解析出错
    if (status > DONE)
    {
        auto peer(conn->getPeerAddress());
        LOG_ERROR << "Parse error:" << req->status2String()
                  << " from " << peer.first << ":" << peer.second << ".";
        currentLoop->delTimer(req->getRequestTimerKey());
        //400:bad request
        if (conn->send(HttpResponse(HttpVersion::Ver_11, HTTP_BAD_REQUEST)) == -1)
        {
            conn->handleClose();
            return;
        }
        conn->setState(DISCONNECTING);
        if (buf.empty())
        {
            conn->handleClose();
        }
    }
    else if (status == DONE)
    {
        auto peer(conn->getPeerAddress());
        LOG_DEBUG << "Parse http request from " << peer.first << ":" << peer.second << " done.";
        currentLoop->delTimer(req->getRequestTimerKey());
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
            req->getHeader("Connection") != "keep-alive" && req->getHeader("connection") != "Keep-Alive" && req->getHeader("connection") != "keep-alive")
        {
            //NOTE:由于webbench、ab压测时都需要服务端主动关闭连接(默认短连接)，所以这里在解析完请求之后主动设置当前
            //连接状态为准备断开状态，以便写事件回调在写完响应到套接字内核缓冲区后能够及时关闭连接；
            conn->setState(DISCONNECTING);
            //写完直接关
            if (buf.empty())
            {
                conn->handleClose();
            }
        }
        else //长连接
        {
            LOG_DEBUG << "Keep alive to:" << peer.first << ":" << peer.second;
            conn->resetBuffer();
            req->resetRequest();
            req->setAliveTimerKey(currentLoop->addTimer(aliveTimeout_, std::bind(&Connection::handleClose, conn.get())));
            LOG_DEBUG << "Set timer,timeout at:" << req->getAliveTimerKey().format();
        }
    }
}
} // namespace net