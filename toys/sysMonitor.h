#ifndef __sys_monitor_h
#define __sys_monitor_h

#include <string>
#include <unordered_map>
#include <functional>
#include "../net/httpServer.h"

namespace toys
{
class SystemMonitor
{
    using ConnectionPtr = net::Server::ConnectionPtr;
    typedef std::vector<std::string> Args;
    //TODO:调用的回调的参数？目前仅仅是command带的参数
    typedef std::function<std::string(const Args &)> Callback;
    typedef std::unordered_map<std::string, Callback> Commands;
    typedef std::unordered_map<std::string, Commands> Modules;

public:
    SystemMonitor(event::EventLoop *loop, const std::vector<int> &ports);
    ~SystemMonitor() = default;
    void run(int numThreads = 1)
    {
        server_.run(numThreads);
    }
    void stop()
    {
        server_.stop();
    }
    //添加一个module/command->callback
    bool add(const std::string &module, const std::string &command, Callback cb);
    //删除一个module/command->callback
    bool remove(const std::string &module, const std::string &command);
    void onResponse(const ConnectionPtr &conn);

private:
    net::HttpServer server_;
    Modules modules_;
};
} // namespace toys

#endif