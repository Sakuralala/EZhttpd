#include <signal.h>
#include <vector>
#include "net/httpServer.h"
#include "event/eventLoop.h"

class IgnSigPipe
{
  public:
    IgnSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
IgnSigPipe ign;
//忽略sigpipe信号
int main()
{
    using event::EventLoop;
    using net::HttpServer;
    std::vector<int> ports{8888};
    EventLoop baseLoop;
    HttpServer server(&baseLoop, ports);
    server.run(4);
    //测试下内存泄漏
    /*
    event::Timer stopTimer(35,std::bind(&net::HttpServer::stop,&server));
    baseLoop.addTimer(stopTimer);
    */
    baseLoop.loop();
}