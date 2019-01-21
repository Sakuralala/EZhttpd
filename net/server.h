#ifndef __server_h
#define __server_h
/**
 * 服务器的抽象；
 ***/
#include <vector>
#include "bases/IOThreadPool.h"
#include "acceptor.h"
namespace event
{
class event::EventLoop;
}
//struct sockaddr_in;
namespace net
{
class HttpRequest;
class HttpResponse;
class Server
{
  public:
    Server(std::vector<int> &ports);
    ~Server();
    void run(int numThreads);
    void stop();
    //新连接建立的回调
    void onConnection(int acceptFd,struct sockaddr_in clientAddr);
    
  private:
    event::IOThreadPool pool_;
    event::EventLoop *loop_;
    Acceptor acceptor_;
    std::vector<int> bindingPorts_;
    bool running_;
};
} // namespace net
#endif // !__server_h