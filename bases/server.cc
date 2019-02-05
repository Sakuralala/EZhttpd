#include <netinet/in.h>
#include <sys/socket.h>
#include "server.h"
#include "logger.h"
#include "eventLoop.h"
#include "connection.h"
#define MAX_CONN 2147483647
namespace net
{
Server::Server(event::EventLoop *loop, const std::vector<int> &ports) : running_(false), total_(0), loop_(loop), acceptor_(loop_), bindingPorts_(ports)
{
    if (!loop_)
        LOG_FATAL << "Create server failed,event loop can't be null.";
    //setReadCallback(std::bind(&net::Server::discardMsg, this, std::placeholders::_1, std::placeholders::_2));
}
Server::~Server()
{
    stop();
}
void Server::discardMsg(const ConnectionPtr &ptr, bases::UserBuffer &buf)
{
    ptr->send("HTTP/1.1 404 Not Found\r\n\r\n", 30);
}
//新连接建立成功后需要将其按照某种策略分配到一个sub reactor中
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
    LOG_INFO << "Total connection:" << total_;
    auto loop = pool_.getNextLoop();
    /*
    EventPtr ev(new event::Event(acceptFd, loop));
    ev->setReadCallback(readCallback_);
    ev->setErrorCallback(errorCallback_);
    ev->setWriteCallback(writeCallback_);
    //TODO:修改可同时打开的文件描述符的最值
    //ref:2
    connected.emplace(acceptFd, ev);
    loop->runInLoop(std::bind(&event::Event::enableAll, ev.get()));
    */
    struct sockaddr_in localAddr;
    socklen_t localAddrLen = sizeof(localAddr);
    if (getsockname(acceptFd, (struct sockaddr *)&localAddr, &localAddrLen))
    {
        LOG_ERROR << "Get sock name error in socket:" << acceptFd;
        return;
    }
    //ref:1
    ConnectionPtr conn(new Connection(loop, acceptFd, localAddr, clientAddr));
    conn->setErrorCallback(errorCallback_);
    conn->setWriteCallback(writeCallback_);
    conn->setReadCallback(readCallback_);
    conn->setCloseCallback(std::bind(&net::Server::delConnection, this, std::placeholders::_1));
    connected_.emplace(acceptFd, conn);
    //修改感兴趣的事件需要在owner线程中进行，因为没有加锁
    loop->runInLoop(std::bind(&net::Connection::eventInit, conn.get()));
    auto peer = conn->getPeerAddress();
    auto local = conn->getLocalAddress();
    //LOG_WARN << "New connection distributed from: "
    //        << peer.first << ":" << peer.second << " to :" << local.first << ":" << local.second;
}
//此函数一般都是在子线程close对应connection时进行回调，即由其他线程调用，为了防止race condition
//所以需要runInLoop
void Server::delConnection(int fd)
{
    //loop_->runInLoop([fd, this]() { this->_delConnection(fd); });
    loop_->runInLoop(std::bind(&net::Server::_delConnection, this, fd));
}
void Server::_delConnection(int fd)
{
    //QUESTION:由server进行描述符的关闭？
    //ANSWER:暂时先这样
    if (::close(fd))
        LOG_ERROR << "Close fd:" << fd << " error:" << strerror(errno);
    else
        LOG_WARN << "Close fd:" << fd;
    //ref:1 没问题 反正close之后就要析构connection了
    //LOG_INFO << "Current reference count:" << connected_[fd].use_count();
    connected_.erase(fd);
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
}
} // namespace net