#ifndef __connection_h
#define __connection_h
/**
 * 连接的抽象，主要就是Event+读写缓冲区+两端地址; 
 * 改为Connection继承自Event似乎更好，可以减少数据成员的冗余，以及回调调用的冗余,但是这样的话如
 * 何传递Server的回调到Connection中?一种解决方法就是不传回调了，直接写死下面需要调用的函数，直接
 * 调用HttpServer::onMessage;    
 * **/
#include <any>
#include <functional>
#include <memory>
#include <utility>
#include <netinet/in.h>
//#include "../bases/circularBuffer.h"
#include "../bases/newCircularBuffer.h"
#include "../event/event.h"
#include "../event/timer.h"
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
class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;
class Connection : public std::enable_shared_from_this<Connection>
{
public:
    typedef std::function<void(bases::UserBuffer &)> WriteCallback;
    //由于读事件读完之后或者是出错后需要发送响应，需要调用Connection::send,故需要这个参数
    typedef std::function<void(const ConnectionPtr &, bases::UserBuffer &)> ReadCallback;
    typedef std::function<void()> Callback;
    typedef std::function<void(const ConnectionPtr &)> CloseCallback;
    Connection(event::EventLoop *loop, int fd, const sockaddr_in &local, const sockaddr_in &peer);
    ~Connection();
    int getFd() const
    {
        return event_.getFd();
    }
    void setReadCallback(ReadCallback cb)
    {
        readCallback_ = std::move(cb);
    }

    void setWriteCallback(WriteCallback cb)
    {
        writeCallback_ = std::move(cb);
    }

    /*
    void setErrorCallback(Callback cb)
    {
        errorCallback_ = std::move(cb);
    }
    */
    void setCloseCallback(CloseCallback cb)
    {
        closeCallback_ = std::move(cb);
    }
    //Event::handleEvent调用的wrapper，作用就是传递用户态缓冲区；
    void handleRead();
    void handleWrite();
    //void handleError();
    void handleClose();
    //server端
    void shutdownWrite();
    int send(const char *msg, int len);
    int send(const std::string &msg);
    int send(const HttpResponse &response);
    void setContext(const std::any &context)
    {
        context_ = context;
    }
    void setState(ConnectionState state)
    {
        state_ = state;
    }
    std::any *getContext()
    {
        return &context_;
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
    bool outEmpty() const
    {
        return out_.empty();
    }
    bool inEmpty() const
    {
        return in_.empty();
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
    //Callback errorCallback_;
    CloseCallback closeCallback_;
    //TODO:改为指向任意类型
    //std::shared_ptr<HttpRequest> context_;
    //内部通过std::unique_ptr管理对象的生命周期，无需操心
    std::any context_;
};
} // namespace net
#endif // !__connection_h