#ifndef __client_h
#define __client_h

#include <memory>
#include <functional>
#include <string>
//#include <sys/socket.h>
#include <netinet/in.h>
#include "../event/eventLoop.h"
#include "../event/event.h"
#include "../bases/newCircularBuffer.h"
namespace net
{
class Connection;
//consider 2 parts: error dealing; retry;
class Client : public Nocopyable
{
public:
    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::function<void(bases::UserBuffer &)> WriteCallback;
    typedef std::function<void(const ConnectionPtr &, bases::UserBuffer &)> ReadCallback;
    typedef std::function<void(const ConnectionPtr &)> Callback;
    Client(event::EventLoop *loop);
    ~Client() = default;
    event::EventLoop *getLoop() const
    {
        return loop_;
    }
    void defaultCloseCallback(const ConnectionPtr &conn);
    bool connect(struct sockaddr_in &peerAddr);
    bool connect(const std::string &peer);
    bool connect(const std::string &peerIP, int port);
    void disconnect();
    void shutdownWrite();
    //before established callbacks
    void connectionEstablished();
    void connectionFailed();
    void setReadCallback(ReadCallback cb)
    {
        readCallback_ = std::move(cb);
    }
    void setCloseCallback(Callback cb)
    {
        closeCallback_ = std::move(cb);
    }
    void setConnectionCallback(Callback cb)
    {
        connectionCallback_ = std::move(cb);
    }
    void setWriteCallback(WriteCallback cb)
    {
        writeCallback_ = std::move(cb);
    }
    std::pair<std::string, uint16_t> getPeerAddress() const;
    std::pair<std::string, uint16_t> getLocalAddress() const;

private:
    bool _connect();
    void connecting();
    void retry();
    void resetConnectingEvent();
    //
    const int MaxRetryTimes = 3;
    bool connected_;
    int retryTimes_;
    event::EventLoop *loop_;
    int fd_;
    struct sockaddr_in peerAddr_;
    struct sockaddr_in localAddr_;
    ConnectionPtr conn_;
    //notify when connection established.
    std::unique_ptr<event::Event> connectingEvent_;
    ReadCallback readCallback_;
    WriteCallback writeCallback_;
    Callback closeCallback_;
    Callback connectionCallback_;
};

} // namespace net

#endif // !__client_h