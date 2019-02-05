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
  HttpServer(event::EventLoop *loop, const std::vector<int> &ports);
  ~HttpServer();
  void onMessage(const ConnectionPtr &conn, bases::CircularBuffer &buf);
  void setRequestTimeout(uint32_t sec)
  {
    requestTimeout_ = sec;
  }
  uint32_t getRequestTimeout() const
  {
    return requestTimeout_;
  }
  void setAliveTimeout(uint32_t sec)
  {
    aliveTimeout_ = sec;
  }
  uint32_t getAliveTimeout(uint32_t sec) const
  {
    return aliveTimeout_;
  }

private:
  //单个请求超时时间
  uint32_t requestTimeout_;
  //长连接保持时间
  uint32_t aliveTimeout_;
};

} // namespace net

#endif // !__httpserver_h