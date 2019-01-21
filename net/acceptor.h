#ifndef __acceptor_h
#define __acceptor_h
/***
 *监听端口类，即主reactor，专门用于监听新连接，一建立新连接就将其分配到一个sub reactor中； 
 * **/
#include <vector>
#include <functional>
#include <utility>
#include <unordered_map>
#include "bases/event.h"
namespace event
{
class EventLoop;
} // namespace event
struct sockaddr_in;
namespace net
{
//TODO:从配置文件中读取需要监听的ip:port;
class Acceptor
{
    //typedef std::vector<event::Event> BindingEventList;
    typedef std::function<void(int,struct sockaddr_in)> ConnectionCallback;
    typedef std::unordered_map< int, event::Event> BindingMap;

  public:
    Acceptor(event::EventLoop *loop);
    ~Acceptor();
    //开始监听
    //TODO:修改ip
    bool listen(int port);
    //有新连接到来时的回调
    void accept(int listenFd);
    //新连接建立后的回调
    void setConnectionCallback(ConnectionCallback cb)
    {
        onConnection_ = std::move(cb);
    }

  private:
    event::EventLoop *loop_;
    //监听的端口对应的event
    //BindingEventList bindingEvents_;
    //监听的端口  对应的监听套接字描述符
    //std::vector<std::pair<int,int>> portsSockets_;
    //端口  Event
    //TODO:改为ip:port->Event
    BindingMap map_;
    ConnectionCallback onConnection_;
};
} // namespace net

#endif // !__acceptor_h
