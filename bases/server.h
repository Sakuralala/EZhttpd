#ifndef __server_h
#define __server_h
/**
 * 服务器的抽象；
 ***/
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include "IOThreadPool.h"
#include "acceptor.h"
#include "circularBuffer.h"
namespace event
{
class EventLoop;
class Event;
} // namespace event
//struct sockaddr_in;
namespace net
{
class Connection;
class HttpRequest;
class HttpResponse;
class Server
{
  typedef std::shared_ptr<Connection> ConnectionPtr;
  typedef std::function<void()> Callback;
  typedef std::function<void(const bases::UserBuffer &)> WriteCallback;
  typedef std::function<void(ConnectionPtr &, const bases::UserBuffer &)> ReadCallback;
  typedef std::shared_ptr<event::Event> EventPtr;

public:
  Server(event::EventLoop *loop, std::vector<int> &ports);
  ~Server();
  void run(int numThreads);
  void stop();
  //新连接建立的回调
  void distributeConnetion(int acceptFd, struct sockaddr_in clientAddr);
  void setReadCallback(ReadCallback cb)
  {
    readCallback_ = std::move(cb);
  }

  void setWriteCallback(WriteCallback cb)
  {
    writeCallback_ = std::move(cb);
  }

  void setErrorCallback(Callback cb)
  {
    errorCallback_ = std::move(cb);
  }

private:
  bool running_;
  event::EventLoop *loop_;
  event::IOThreadPool pool_;
  Acceptor acceptor_;
  std::vector<int> bindingPorts_;
  //当前连接的套接字描述符->对于的Event
  std::unordered_map<int, EventPtr> connected;
  readCallback readCallback_;
  writeCallback writeCallback_;
  Callback errorCallback_;
};
} // namespace net
#endif // !__server_h