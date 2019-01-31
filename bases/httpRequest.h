#ifndef __httprequest_h
#define __httprequest_h
/**
 * http请求的抽象； 
 * **/
#include <string>
#include <unordered_map>
#include <memory>
#include "timer.h"
#include "circularBuffer.h"
#include "httpUtils.h"
namespace net
{
class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;
class HttpRequest
{
  public:
    HttpRequest(const ConnectionPtr &conn, uint64_t secs, event::Timer::TimeoutCallback cb);
    ~HttpRequest();
    ParseStatus parse(bases::UserBuffer &buf);
    std::string status2String() const
    {
        return StatusString[status_];
    }

    std::string getHeader(const std::string &str)
    {
        if (headers.count(str))
            return headers[str];
        return "";
    }
    HttpVersion getVersion() const
    {
        return version_;
    }
    std::string getRequestPath() const
    {
        return path_;
    }
    const event::Timer &getTimer() const
    {
        return timer_;
    }
    void setTimer(uint64_t secs)
    {
        timer_.setTime(secs);
    }

  private:
    std::string path_;
    std::string url_;
    HttpMethod method_;
    HttpVersion version_;
    //当前解析状态
    ParseStatus status_;
    //头部
    std::unordered_map<std::string, std::string> headers;
    //正文
    std::string content_;
    ParseStatus parseRequestLine(bases::UserBuffer &buf);
    ParseStatus parseHeader(bases::UserBuffer &buf);
    ParseStatus parseContent(bases::UserBuffer &buf);
    //方便请求找到其上层connection对象
    std::weak_ptr<Connection> owner_;
    //每个请求需要一个定时器以便超时关闭连接
    event::Timer timer_;
};
} // namespace net

#endif // !__httprequest_h