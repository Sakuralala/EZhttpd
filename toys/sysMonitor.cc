#include "sysMonitor.h"
#include "../net/httpRequest.h"
#include "../net/httpResponse.h"
#include "../net/connection.h"
#include "../net/httpUtils.h"
#include "../log/logger.h"
#include "../bases/utils.h"
using net::HttpRequest;
using net::HttpResponse;
using net::HttpVersion;
using net::ConnectionState::DISCONNECTING;
using net::ParseStatus::DONE;
namespace toys
{
SystemMonitor::SystemMonitor(event::EventLoop *loop, const std::vector<int> &ports) : server_(loop, ports)
{
    server_.setResponseCallback(std::bind(&toys::SystemMonitor::onResponse, this, std::placeholders::_1));
}
bool SystemMonitor::add(const std::string &module, const std::string &command, Callback cb)
{
    if (modules_.find(module) != modules_.end())
        return false;
    Commands cmd;
    cmd.emplace(command, cb);
    modules_.emplace(module, std::move(cmd));
    return true;
}
bool SystemMonitor::remove(const std::string &module, const std::string &command)
{
    auto ite = modules_.find(module);
    if (ite == modules_.end())
        return false;
    return ite->second.erase(command) == 1;
}
void SystemMonitor::onResponse(const ConnectionPtr &conn)
{
    auto currentLoop = conn->getLoop();
    HttpRequest *req = std::any_cast<HttpRequest>(conn->getContext());
    auto status = req->status();
    if (status > DONE)
    {
        auto peer(conn->getPeerAddress());
        LOG_ERROR << "Parse error:" << req->status2String()
                  << " from " << peer.first << ":" << peer.second << ".";
        currentLoop->delTimer(req->requestTimerKey());
        //400:bad request
        if (conn->send(HttpResponse(HttpVersion::Ver_11, HTTP_BAD_REQUEST)) == -1)
        {
            conn->handleClose();
            return;
        }
        conn->setState(DISCONNECTING);
        if (conn->outEmpty())
        {
            conn->handleClose();
        }
    }
    else if (status == DONE)
    {
        auto path(req->path());
        if (path == "/")
        {
            if (conn->send(HttpResponse(HttpVersion::Ver_11, path)) == -1)
            {
                conn->handleClose();
                return;
            }
        }
        else
        {
            std::vector<std::string> res(bases::split(path, '/'));
            if (res.size() <= 1)
            {
                if (conn->send(HttpResponse(HttpVersion::Ver_11, "/")) == -1)
                {
                    conn->handleClose();
                    return;
                }
            }
            else //module/command(/arg1/arg2....)
            {
                auto ite1 = modules_.find(res[0]);
                if (ite1 == modules_.end())
                {
                    if (conn->send(HttpResponse(HttpVersion::Ver_11, "module_not_found.html")) == -1)
                    {
                        conn->handleClose();
                        return;
                    }
                }
                auto ite2 = ite1->second.find(res[1]);
                if (ite2 == ite1->second.end())
                {
                    if (conn->send(HttpResponse(HttpVersion::Ver_11, "command_not_found.html")) == -1)
                    {
                        conn->handleClose();
                        return;
                    }
                }
                std::string rep = ite2->second(std::vector<std::string>(res.begin() + 2, res.end()));
                conn->send("HTTP/1.1 200 OK\r\n");
                conn->send("Server:EZHttpd\r\nDate:" + event::Timer(0).format() + "\r\nContent-Length:" + std::to_string(rep.size()) + "\r\nContent-Type:text/plain\r\n\r\n");
                conn->send(rep + "\r\n");
            }
        }

        //不管长连接了
        if (conn->outEmpty())
            conn->handleClose();
    }
}
} // namespace toys