#include <vector>
#include "httpServer.h"
#include "eventLoop.h"

int main()
{
    using event::EventLoop;
    using net::HttpServer;
    std::vector<int> ports{8888};
    EventLoop baseLoop;
    HttpServer server(&baseLoop,ports);
    server.run(4);
    baseLoop.loop();
}