#ifndef __httprequest_h
#define __httprequest_h
/**
 * http请求的抽象； 
 * **/
#include <string>
#include <unordered_map>
#include <memory>
#include "httpUtils.h"
#include "../event/timer.h"
#include "../bases/circularBuffer.h"
namespace net
{
class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;
class HttpRequest
{
  public:
    HttpRequest(const ConnectionPtr &conn);
    ~HttpRequest();
    ParseStatus parse(bases::UserBuffer &buf);
    std::string status2String() const
    {
        return StatusString[status_];
    }

    std::string getHeader(const std::string &str)
    {
        if (headers_.count(str))
            return headers_[str];
        return "";
    }
    HttpMethod getMethod() const
    {
        return method_;
    }
    ParseStatus getStatus() const
    {
        return status_;
    }
    HttpVersion getVersion() const
    {
        return version_;
    }
    std::string getRequestPath() const
    {
        return path_;
    }
    const event::Timer &getRequestTimer() const
    {
        return requestTimer_;
    }
    void setRequestTimer(uint64_t secs,event::Timer::TimeoutCallback cb)
    {
        requestTimer_.setTime(secs);
        requestTimer_.setCallback(std::move(cb));
    }
    const event::Timer &getAliveTimer() const
    {
        return aliveTimer_;
    }
    void setAliveTimer(uint64_t secs,event::Timer::TimeoutCallback cb)
    {
        aliveTimer_.setTime(secs);
        aliveTimer_.setCallback(std::move(cb));
    }

    //重置http请求，方便后续复用
    void resetRequest();

  private:
    std::string path_;
    std::string url_;
    HttpMethod method_;
    HttpVersion version_;
    //当前解析状态
    ParseStatus status_;
    //头部
    std::unordered_map<std::string, std::string> headers_;
    //正文
    std::string content_;
    ParseStatus parseMethod(bases::UserBuffer &buf);
    ParseStatus parseURL(bases::UserBuffer &buf);
    ParseStatus parseProtocol(bases::UserBuffer &buf);
    ParseStatus parseHeader(bases::UserBuffer &buf);
    ParseStatus parseContent(bases::UserBuffer &buf);
    //方便请求找到其上层connection对象
    std::weak_ptr<Connection> owner_;
    //每个请求需要一个定时器以便超时关闭连接
    event::Timer requestTimer_;
    //长连接定时器
    event::Timer aliveTimer_;
};
} // namespace net

#endif // !__httprequest_h