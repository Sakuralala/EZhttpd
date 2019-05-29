#ifndef __server_h
#define __server_h
/**
 * 服务器的抽象；
 ***/
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include "acceptor.h"
//#include "../bases/circularBuffer.h"
#include "../bases/newCircularBuffer.h"
#include "../event/IOThreadPool.h"
#include "../event/timer.h"
#include "../event/eventLoop.h"

namespace event
{
//class EventLoop;
class Event;
} // namespace event
//struct sockaddr_in;
namespace net
{
class Connection;
class Server
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;
  typedef std::function<void()> Callback;
  typedef std::function<void(bases::UserBuffer &)> WriteCallback;
  typedef std::function<void(const ConnectionPtr &, bases::UserBuffer &)> ReadCallback;
  typedef std::function<void(const ConnectionPtr &)> CloseCallback;
  //typedef std::shared_ptr<event::Event> EventPtr;

  event::EventLoop *getLoop()
  {
    return loop_;
  }
  void setReadCallback(ReadCallback cb)
  {
    readCallback_ = std::move(cb);
  }
  void setCloseCallback(CloseCallback cb)
  {
    closeCallback_ = std::move(cb);
  }
  void setWriteCallback(WriteCallback cb)
  {
    writeCallback_ = std::move(cb);
  }

  void setErrorCallback(Callback cb)
  {
    errorCallback_ = std::move(cb);
  }

  Server(event::EventLoop *loop, const std::vector<int> &ports, int interval = 30);
  virtual ~Server();
  void run(int numThreads);
  void stop();
  //新连接建立的回调
  void distributeConnetion(int acceptFd, const struct sockaddr_in &clientAddr);
  //默认读完后会将数据全部丢弃
  void defaultReadCallback(const ConnectionPtr &conn, bases::UserBuffer &buf);
  //default close callback
  void delConnection(const ConnectionPtr &conn);
  //idel function, just show current connetion number.
  //run every 30s.
  void connectionInfoShow() const;

protected:
  //当前连接的套接字描述符->对于的Event
  std::unordered_map<int, ConnectionPtr> connected_;

private:
  //具体实现
  void _delConnection(int fd);
  bool running_;
  int infoShowInterval_;
  long long total_;
  //最大维持连接数
  int maxConcurrentNumber_;
  event::EventLoop *loop_;
  event::IOThreadPool pool_;
  event::TimerKey timerKey_;
  Acceptor acceptor_;
  std::vector<int> bindingPorts_;
  ReadCallback readCallback_;
  //TODO:delete,写事件回调最基本的就是在可写时将用户态写缓冲区中的内容写到内核态缓冲区中
  //这个在Connection::handleWrite中已经完成了，后续可以不需要其他操作；
  WriteCallback writeCallback_;
  CloseCallback closeCallback_;
  //TODO:delete,错误的处理流程已经包含在读写回调中了
  Callback errorCallback_;
};
} // namespace net
#endif // !__server_h