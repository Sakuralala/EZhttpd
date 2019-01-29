#ifndef __httprequest_h
#define __httprequest_h
/**
 * http请求的抽象； 
 * **/
#include <string>
#include <unordered_map>
#include "circularBuffer.h"

namespace net
{
enum HttpMethod
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE
};
enum HttpVersion
{
    Ver_10,
    Ver_11
};
enum ParseStatus
{
    REQUEST_LINE,
    HEADERS,
    CONTENT,
    DONE
};
class HttpRequest
{
  public:
    HttpRequest():status_(REQUEST_LINE) {}
    ~HttpRequest() = default;
    bool parse(bases::UserBuffer &buf);

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
    bool parseRequestLine(bases::UserBuffer &buf);
    bool parseHeader(bases::UserBuffer &buf);
    bool parseContent(bases::UserBuffer &buf);
};
} // namespace net

#endif // !__httprequest_h