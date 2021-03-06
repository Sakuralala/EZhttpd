#include <netinet/in.h>
#include <sys/socket.h>
#include "server.h"
#include "connection.h"
#include "../event/timer.h"
#include "../event/eventLoop.h"
#include "../log/logger.h"
#define MAX_CONN 2147483647
namespace net
{
Server::Server(event::EventLoop *loop, const std::vector<int> &ports, int interval) : running_(false), infoShowInterval_(interval), total_(0), maxConcurrentNumber_(0), loop_(loop), timerKey_(nullptr), acceptor_(loop_), bindingPorts_(ports)
{
    if (!loop_)
        LOG_FATAL << "Create server failed,event loop can't be null.";

    timerKey_ = loop_->addTimer(infoShowInterval_, std::bind(&net::Server::connectionInfoShow, this), 0);
    setReadCallback(std::bind(&net::Server::defaultReadCallback, this, std::placeholders::_1, std::placeholders::_2));
    setCloseCallback(std::bind(&net::Server::delConnection, this, std::placeholders::_1));
}
Server::~Server()
{
    stop();
}
void Server::defaultReadCallback(const ConnectionPtr &conn, bases::UserBuffer &buf)
{
    //LOG_INFO << "New read event in socket: " << conn->getFd()<<",data:"<<buf.getAll();
    conn->send(buf.getAll());
    buf.retrieve(buf.size());
    conn->shutdownWrite();
}
void Server::connectionInfoShow() const
{
    LOG_INFO << "Current connection number:" << connected_.size() << ", max concurrent connection number:" << maxConcurrentNumber_ << ", total connection number:" << total_;
}

//新连接建立成功后需要将其按照某种策略分配到一个sub reactor中
//还有一种做法就是主reactor只传递新建立的套接字描述符给sub reactor，后续的创建Connection由sub reactor来做，这样的话，主reactor就不知道当前有多少个具体连接了 而且需要另外设置Connection的回调
void Server::distributeConnetion(int acceptFd, const struct sockaddr_in &clientAddr)
{
    /**
     * 大概思路是这样:
     * 1.Acceptor:
     * Acceptor负责监听新连接，并提供了一个新连接建立后进行获取的接口:
     * Acceptor::accept,内部会调用accept4来获取已连接套接字描述符，并且还会调用Acceptor提供
     * 另一个接口:Accept::setConnetionCallback,这个是accept后需要采取的动作，不由Acceptor
     * 决定；另外，Acceptor还提供了一个负责监听的接口:Acceptor::listen,此函数负责根据提供的
     * ip:port来进行listen，内部会调用listen;
     * 2.Server:
     * 这里的server其实是指基于tcp协议层面的server，它内部拥有一个Acceptor,用于监听并接收
     * 新的连接，Acceptor::setConnetionCallback由Server负责设置为
     * Server::distributeConnetion,内部会采取某种策略将新建立的连接分配到某个sub reactor中
     * 进行后续事件的处理； 
     * 另外，需要为后续的事件处理提供回调，这里Server::set*Callback提供了3个接口,用于设置回
     * 调;根据实际的应用层协议来设置这些回调；
     * 3.HttpServer:
     * 实际应用层的server，负责实际的处理工作；
     * **/
    loop_->assertInOwnerThread();
    total_ += 1;
    LOG_DEBUG << "Total connection:" << total_;
    auto loop = pool_.getNextLoop();
    struct sockaddr_in localAddr;
    socklen_t localAddrLen = sizeof(localAddr);
    if (getsockname(acceptFd, (struct sockaddr *)&localAddr, &localAddrLen))
    {
        LOG_ERROR << "Get sock name error in socket:" << acceptFd;
        return;
    }
    ConnectionPtr conn(new Connection(loop, acceptFd, localAddr, clientAddr));
    //conn->setErrorCallback(errorCallback_);
    conn->setWriteCallback(writeCallback_);
    conn->setReadCallback(readCallback_);
    conn->setCloseCallback(closeCallback_);
    //conn->setCloseCallback(std::bind(&net::Server::delConnection, this, std::placeholders::_1));
    connected_.emplace(acceptFd, conn);
    if (maxConcurrentNumber_ < connected_.size())
    {
        maxConcurrentNumber_ = connected_.size();
        LOG_INFO << "Current cocurrent connection number:" << maxConcurrentNumber_;
    }
    //修改感兴趣的事件需要在owner线程中进行，因为没有加锁
    loop->runInLoop(std::bind(&net::Connection::eventInit, conn.get()));
}
//此函数一般都是在子线程close对应connection时进行回调，即由其他线程调用，为了防止race condition
//所以需要runInLoop
void Server::delConnection(const ConnectionPtr &conn)
{
    //loop_->runInLoop([fd, this]() { this->_delConnection(fd); });
    loop_->runInLoop(std::bind(&net::Server::_delConnection, this, conn->getFd()));
}
void Server::_delConnection(int fd)
{
    connected_.erase(fd);
    if (!connected_.size())
        LOG_DEBUG << "No connetion now.";
}

void Server::run(int numThreads)
{
    loop_->assertInOwnerThread();
    if (numThreads <= 0)
    {
        LOG_ERROR << "Server run error:threads number less than 0.";
        return;
    }
    //1.开启监听
    for (auto port : bindingPorts_)
        acceptor_.listen(port);
    acceptor_.setConnectionCallback(
        std::bind(
            &net::Server::distributeConnetion, this, std::placeholders::_1, std::placeholders::_2));
    running_ = true;
    //2.启动sub reactor
    pool_.run(numThreads);
}
void Server::stop()
{
    loop_->assertInOwnerThread();
    if (running_)
    {
        running_ = false;
        pool_.stop();
    }
    for (auto &elem : connected_)
        ::close(elem.first);
    loop_->quit();
}
} // namespace net