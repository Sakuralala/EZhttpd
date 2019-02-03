#include <signal.h>
#include <vector>
#include "httpServer.h"
#include "eventLoop.h"

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
    baseLoop.loop();
}