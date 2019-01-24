#ifndef __connection_h
#define __connection_h
/**
 * 连接的抽象，主要就是Event+读写缓冲区+两端地址; 
 * **/
#include <functional>
#include <netinet/in.h>
#include "event.h"
namespace event
{
class EventLoop;
}
namespace net
{
class Connection
{
    typedef std::function<void()> Callback;

  public:
    Connection(event::EventLoop *loop, int fd, const sockaddr_in &local, const sockaddr_in &peer);
    ~Connection();
    void setReadCallback(Callback cb)
    {
        readCallback_ = std::move(cb);
    }

    void setWriteCallback(Callback cb)
    {
        writeCallback_ = std::move(cb);
    }

    void setErrorCallback(Callback cb)
    {
        errorCallback_ = std::move(cb);
    }
    //Event::handleEvent调用的wrapper，作用就是传递用户态缓冲区；
    void handleRead();
    void handleWrite();
    void handleError();

  private:
    event::EventLoop *loop_;
    event::Event event_;
    //TODO:Buffer
    sockaddr_in localAddress_;
    sockaddr_in peerAddress_;
    //事件就绪时实际调用的回调
    Callback readCallback_;
    Callback writeCallback_;
    Callback errorCallback_;
};
} // namespace net
#endif // !__connection_h