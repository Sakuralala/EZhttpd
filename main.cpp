#include <signal.h>
#include <vector>
#include <iostream>
#include "toys/memcachedServer.h"
#include "net/httpServer.h"
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

int main()
{
  using event::EventLoop;
  using net::HttpServer;
  using toys::MemcachedServer;
  std::vector<int> ports{8888};
  EventLoop baseLoop;
  //HttpServer server(&baseLoop, ports);
  MemcachedServer server(&baseLoop,ports);
  server.run(4);
  //测试下内存泄漏
  //baseLoop.addTimer(40, std::bind(&net::HttpServer::stop, &server));
  baseLoop.loop();
  LOG_INFO << "Server stopped.";
}