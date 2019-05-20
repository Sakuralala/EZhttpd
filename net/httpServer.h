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
class CharCircularBuffer;
}
namespace net
{
class HttpServer : public Server
{
  typedef std::function<void(const ConnectionPtr &)> ResponseCallback;

public:
  HttpServer(event::EventLoop *loop, const std::vector<int> &ports);
  ~HttpServer();
  void onClose(const ConnectionPtr &conn);
  void onMessage(const ConnectionPtr &conn, bases::CharCircularBuffer &buf);
  void onResponse(const ConnectionPtr &conn);
  void setResponseCallback(ResponseCallback cb)
  {
    responseCallback_ = std::move(cb);
  }
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
  //解析完请求后如何发送响应的回调
  ResponseCallback responseCallback_;
};

} // namespace net

#endif // !__httpserver_h