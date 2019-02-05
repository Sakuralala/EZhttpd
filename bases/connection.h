#ifndef __connection_h
#define __connection_h
/**
 * 连接的抽象，主要就是Event+读写缓冲区+两端地址; 
 * **/
#include <functional>
#include <memory>
#include <utility>
#include <netinet/in.h>
#include "circularBuffer.h"
#include "event.h"
#include "timer.h"
namespace event
{
class EventLoop;
}
namespace net
{
class HttpRequest;
class HttpResponse;
enum ConnectionState
{
    CONNECTED,
    DISCONNECTING,
    DISCONNECTED
};
class Connection : public std::enable_shared_from_this<Connection>
{
  public:
    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::function<void(bases::UserBuffer &)> WriteCallback;
    //由于读事件读完之后或者是出错后需要发送响应，需要调用Connection::send,故需要这个参数
    typedef std::function<void(const ConnectionPtr &, bases::UserBuffer &)> ReadCallback;
    typedef std::function<void()> Callback;
    typedef std::function<void(int)> CloseCallback;
    Connection(event::EventLoop *loop, int fd, const sockaddr_in &local, const sockaddr_in &peer);
    ~Connection();
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
    void setCloseCallback(CloseCallback cb)
    {
        closeCallback_ = std::move(cb);
    }
    //Event::handleEvent调用的wrapper，作用就是传递用户态缓冲区；
    void handleRead();
    void handleWrite();
    void handleError();
    void handleClose();
    int send(const char *msg, int len);
    int send(const std::string &msg);
    int send(const HttpResponse &response);
    void setContext(const std::shared_ptr<HttpRequest> &context)
    {
        context_ = context;
    }
    void setState(ConnectionState state)
    {
        state_ = state;
    }
    std::shared_ptr<HttpRequest> getContext()
    {
        return context_;
    }
    event::EventLoop *getLoop() const
    {
        return loop_;
    }
    //enableAll+tie
    void eventInit();
    std::pair<std::string, uint16_t> getPeerAddress() const;
    std::pair<std::string, uint16_t> getLocalAddress() const;
    void resetBuffer()
    {
        in_.reset();
        out_.reset();
    }

  private:
    ConnectionState state_;
    event::EventLoop *loop_;
    event::Event event_;
    bases::UserBuffer in_;
    bases::UserBuffer out_;
    sockaddr_in localAddress_;
    sockaddr_in peerAddress_;
    //事件就绪时实际调用的回调
    ReadCallback readCallback_;
    WriteCallback writeCallback_;
    Callback errorCallback_;
    CloseCallback closeCallback_;
    //TODO:改为指向任意类型
    std::shared_ptr<HttpRequest> context_;
};
} // namespace net
#endif // !__connection_h