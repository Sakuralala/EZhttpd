#include <algorithm>
#include <string.h>
#include "httpRequest.h"
#include "connection.h"
#include "logger.h"
namespace net
{
HttpRequest::HttpRequest(const ConnectionPtr &conn, uint64_t secs, event::Timer::TimeoutCallback cb) : status_(REQUEST_LINE),
                                                                                                       owner_(conn), timer_(secs, std::move(cb))
{
}
ParseStatus HttpRequest::parse(bases::UserBuffer &buf)
{
    if (status_ == REQUEST_LINE)
        return parseRequestLine(buf);
    else if (status_ == HEADERS)
        return parseHeader(buf);
    else if (status_ == CONTENT)
        return parseContent(buf);

    return status_;
}
//解析请求行
ParseStatus HttpRequest::parseRequestLine(bases::UserBuffer &buf)
{
    auto pos = buf.find(' ');
    if (!pos)
        return REQUEST_LINE;
    int len = buf.length(buf.begin(), pos);
    /*****************解析请求方法***************************/
    if (len == 3)
    {
        if (buf.compare("GET", len))
            method_ = GET;
        else if (buf.compare("PUT", len))
            method_ = PUT;
        else
        {
            LOG_ERROR << "Unknown method.";
            status_ = REQUEST_LINE_ERROR;
            return REQUEST_LINE_ERROR;
        }
    }
    else if (len == 4)
    {
        if (buf.compare("HEAD", len))
            method_ = HEAD;
        else if (buf.compare("POST", len))
            method_ = POST;
        else
        {
            LOG_ERROR << "Unknown method.";
            status_ = REQUEST_LINE_ERROR;
            return REQUEST_LINE_ERROR;
        }
    }
    else if (len == 6)
    {
        if (buf.compare("DELETE", len))
            method_ = DELETE;
        else
        {
            LOG_ERROR << "Unknown method.";
            status_ = REQUEST_LINE_ERROR;
            return REQUEST_LINE_ERROR;
        }
    }
    else
    {
        LOG_ERROR << "Unknown method.";
        status_ = REQUEST_LINE_ERROR;
        return REQUEST_LINE_ERROR;
    }
    buf.retrieve(len + 1);
    /****************************解析url********************************/
    pos = buf.find(' ');
    if (!pos)
        return REQUEST_LINE;
    //move sematic
    url_ = buf.getMsg(buf.begin(), pos);
    //move sematic
    path_ = url_.substr(url_.find('/'));
    buf.retrieve(len + 1);

    /******************************解析协议版本*****************************/
    pos = buf.find('/');
    if (!pos)
        return REQUEST_LINE;
    if (!buf.compare("HTTP", 4))
    {
        LOG_ERROR << "Unknown protocol.";
        status_ = REQUEST_LINE_ERROR;
        return REQUEST_LINE_ERROR;
    }
    //"http/"
    buf.retrieve(5);
    pos = buf.findCRLF();
    if (!pos)
        return REQUEST_LINE;
    //len = buf.length(buf.begin(), pos);
    if (buf.compare("1.1", 3))
        version_ = Ver_11;
    else if (buf.compare("1.0", 3))
        version_ = Ver_10;
    else
    {
        LOG_ERROR << "Unknown http version.";
        status_ = REQUEST_LINE_ERROR;
        return REQUEST_LINE_ERROR;
    }
    //example:1.1\r\n
    buf.retrieve(5);
    status_ = HEADERS;
    /**************请求行解析完成，进行请求头的解析****************************/
    return parseHeader(buf);
}
ParseStatus HttpRequest::parseHeader(bases::UserBuffer &buf)
{
    while (true)
    {
        auto pos = buf.findCRLF();
        if (!pos)
            return HEADERS;
        auto header(buf.getMsg(buf.begin(), pos));
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
        headers.emplace(header.substr(0, dec), header.substr(dec + 1));
    }
    status_ = CONTENT;
    return parseContent(buf);
}
ParseStatus HttpRequest::parseContent(bases::UserBuffer &buf)
{
    //TODO:解析请求正文
    status_ = DONE;
    return DONE;
}
} // namespace net