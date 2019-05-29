#include <signal.h>
#include <vector>
#include <iostream>
//#include "toys/memcachedServer.h"
#include "net/server.h"
#include "event/eventLoop.h"
#include "bases/thread.h"
#include "log/logger.h"
using std::cout;
using std::endl;
class IgnSigPipe
{
public:
    IgnSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
//忽略sigpipe信号
IgnSigPipe ign;

int main(int argc, char *argv[])
{
    using event::EventLoop;
    using net::Server;
    if (argc == 2)
    {
        std::vector<int> ports{std::stoi(argv[1])};
        EventLoop baseLoop;
        Server server(&baseLoop, ports);
        server.run(4);
        //测试下内存泄漏
        //baseLoop.addTimer(35, std::bind(&net::HttpServer::stop, &server));
        baseLoop.loop();
        LOG_INFO << "Server stopped.";
    }
    else
    {
        LOG_INFO << "Usage:./server_test port";
    }
}