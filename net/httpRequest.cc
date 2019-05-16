#include <algorithm>
#include <string.h>
#include "httpRequest.h"
#include "connection.h"
#include "../log/logger.h"
namespace net
{
HttpRequest::HttpRequest(const ConnectionPtr &conn) : status_(REQUEST_METHOD),
                                                      owner_(conn), requestTimerKey_(nullptr), aliveTimerKey_(nullptr)
{
}
HttpRequest::~HttpRequest() = default;
ParseStatus HttpRequest::parse(bases::UserBuffer &buf)
{
    //仅仅为了长连接而使用
    if (status_ == WAIT)
        status_ = REQUEST_METHOD;
    if (status_ == REQUEST_METHOD)
        return parseMethod(buf);
    else if (status_ == REQUEST_URL)
        return parseURL(buf);
    else if (status_ == PROTOCOL)
        return parseProtocol(buf);
    else if (status_ == HEADERS)
        return parseHeader(buf);
    else if (status_ == CONTENT)
        return parseContent(buf);

    return status_;
}
ParseStatus HttpRequest::parseMethod(bases::UserBuffer &buf)
{
    LOG_DEBUG << "Parsing request method.....";
    auto pos = buf.find(' ');
    if (pos == buf.end())
        return REQUEST_METHOD;
    auto len = pos - buf.begin();
    //assert len != -1
    /*
    if (len == -1)
    {
        LOG_ERROR << "Read Circular buffer error.";
        status_ = REQUEST_METHOD_ERROR;
        return REQUEST_METHOD_ERROR;
    }
    */
    auto method(buf.getMsg(buf.begin(), len));
    /*****************解析请求方法***************************/
    if (len == 3)
    {
        if (method == "GET")
            method_ = GET;
        else if (method == "PUT")
            method_ = PUT;
        else
        {
            LOG_ERROR << "Unknown method:" << method;
            status_ = REQUEST_METHOD_ERROR;
            return REQUEST_METHOD_ERROR;
        }
    }
    else if (len == 4)
    {
        if (method == "HEAD")
            method_ = HEAD;
        else if (method == "POST")
            method_ = POST;
        else
        {
            LOG_ERROR << "Unknown method:" << method;
            status_ = REQUEST_METHOD_ERROR;
            return REQUEST_METHOD_ERROR;
        }
    }
    else if (len == 6)
    {
        if (method == "DELETE")
            method_ = DELETE;
        else
        {
            LOG_ERROR << "Unknown method:" << method;
            status_ = REQUEST_METHOD_ERROR;
            return REQUEST_METHOD_ERROR;
        }
    }
    else
    {
        LOG_ERROR << "Unknown method:" << method;
        status_ = REQUEST_METHOD_ERROR;
        return REQUEST_METHOD_ERROR;
    }
    buf.retrieve(len + 1);
    status_ = REQUEST_URL;
    return parseURL(buf);
}
ParseStatus HttpRequest::parseURL(bases::UserBuffer &buf)
{
    LOG_DEBUG << "Parsing request url.....";
    /****************************解析url********************************/
    auto pos = buf.find(' ');
    if (pos == buf.end())
        return REQUEST_URL;
    //move sematic
    url_ = buf.getMsg(buf.begin(), pos);
    auto len = pos - buf.begin();
    //move sematic
    auto slashPos = url_.find('/');
    if (slashPos == std::string::npos)
    {
        LOG_ERROR << "Unknown requested path." << url_;
        status_ = REQUEST_URL_ERROR;
        return status_;
    }
    path_ = url_.substr(slashPos);
    buf.retrieve(len + 1);
    status_ = PROTOCOL;
    return parseProtocol(buf);
}
ParseStatus HttpRequest::parseProtocol(bases::UserBuffer &buf)
{
    LOG_DEBUG << "Parsing protocol.....";
    /******************************解析协议版本*****************************/
    auto pos = buf.findCRLF();
    if (pos == buf.end())
        return PROTOCOL;
    auto protocol(buf.getMsg(buf.begin(), 5));
    if (protocol != "HTTP/")
    {
        LOG_ERROR << "Unknown protocol:" << protocol;
        status_ = PROTOCOL_ERROR;
        return status_;
    }
    //"http/"
    buf.retrieve(5);
    auto version(buf.getMsg(buf.begin(), 3));
    if (version == "1.1")
        version_ = Ver_11;
    else if (version == "1.0")
        version_ = Ver_10;
    else
    {
        LOG_ERROR << "Unsupported http version." << version;
        status_ = PROTOCOL_ERROR;
        return status_;
    }
    //example:1.1\r\n
    buf.retrieve(5);
    status_ = HEADERS;
    /**************请求行解析完成，进行请求头的解析****************************/
    return parseHeader(buf);
}
ParseStatus HttpRequest::parseHeader(bases::UserBuffer &buf)
{
    LOG_DEBUG << "Parsing header.....";
    while (true)
    {
        auto pos = buf.findCRLF();
        if (pos == buf.end())
            return HEADERS;
        auto header(buf.getMsg(buf.begin(), pos));
        LOG_DEBUG << "Header:" << header << ",length:" << header.length() << "\n";
        //+2:\r\n
        buf.retrieve(header.length() + 2);
        if (!header.length())
            break;
        auto dec = header.find(':');
        if (dec == std::string::npos)
        {
            LOG_ERROR << "Invalid http header.";
            status_ = HEADERS_ERROR;
            return HEADERS_ERROR;
        }
        //FIXME:可能存在空格
        //FIXED:删除了可能多余的空格
        //TODO:使用stl内置的find_if算法
        auto n1 = dec - 1;
        for (; n1 > 0; --n1)
        {
            if (header[n1] != ' ')
                break;
        }
        auto n2 = dec + 1;
        for (; n2 < header.size(); ++n2)
        {
            if (header[n2] != ' ')
                break;
        }
        headers_.emplace(header.substr(0, n1 + 1), header.substr(n2));
    }
    status_ = CONTENT;
    return parseContent(buf);
}
ParseStatus HttpRequest::parseContent(bases::UserBuffer &buf)
{
    if (headers_.find("Content-Length") != headers_.end() || headers_.find("content-length") != headers_.end())
    {
        int len = std::stoi(headers_["Content-Length"]);
        //解析请求正文
        if (buf.size() < len)
            return status_;
        else
            content_ = std::move(buf.getMsg(buf.begin(), len));
    }
    status_ = DONE;
    return status_;
}
void HttpRequest::resetRequest()
{
    headers_.clear();
    status_ = WAIT;
}
} // namespace net