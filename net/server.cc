#include <netinet/in.h>
#include "server.h"
#include "bases/logger.h"
#include "bases/eventLoop.h"

namespace net
{
Server::Server(std::vector<int> &ports)
{
}
Server::~Server() = default;
//新连接建立成功后需要将其按照某种策略分配到一个sub reactor中
void Server::onConnection(int acceptFd, struct sockaddr_in clientAddr)
{
    auto loop = pool_.getNextLoop();
    //TODO:创建一个channel对象并将读写回调绑定，注意此处的读写回调需要有HttpRequest、HttpResponse参数；

    loop->runInLoop();
}
void Server::run(int numThreads)
{
    if (numThreads <= 0)
    {
        LOG_ERROR << "Server run error:threads number less than 0.";
        return;
    }
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
}
} // namespace net