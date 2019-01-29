#ifndef __httpserver_h
#define __httpserver_h

/**
 * 就是server的简单封装，只是为了提供http层面的回调。 
 * **/
#include <vector>
#include "server.h"
namespace event
{
class EventLoop;
}
namespace bases
{
class CircularBuffer;
}
namespace net
{
class HttpServer : public Server
{
  public:
    HttpServer(event::EventLoop *loop, std::vector<int> &ports);
    ~HttpServer();
    void onMessage(ConnectionPtr &conn, bases::CircularBuffer &buf);

  private:
};

} // namespace net

#endif // !__httpserver_h