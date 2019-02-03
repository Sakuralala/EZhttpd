#include <algorithm>
#include <string.h>
#include "httpRequest.h"
#include "connection.h"
#include "logger.h"
namespace net
{
HttpRequest::HttpRequest(const ConnectionPtr &conn, uint64_t secs, const event::Timer::TimeoutCallback &cb) : status_(REQUEST_LINE),
                                                                                                              owner_(conn),
                                                                                                              requestTimer_(secs, cb),
                                                                                                              aliveTimer_(0, cb)
{
}
HttpRequest::~HttpRequest() = default;
ParseStatus HttpRequest::parse(bases::UserBuffer &buf)
{
    //仅仅为了长连接而使用
    if (status_ == WAIT)
        status_ = REQUEST_LINE;
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
    //LOG_INFO << "Parsing request line.....";
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
            LOG_ERROR << "Unknown method." << buf.getMsg(buf.begin(), pos);
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
            LOG_ERROR << "Unknown method." << buf.getMsg(buf.begin(), pos);
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
            LOG_ERROR << "Unknown method:" << buf.getMsg(buf.begin(), pos);
            status_ = REQUEST_LINE_ERROR;
            return REQUEST_LINE_ERROR;
        }
    }
    else
    {
        LOG_ERROR << "Unknown method." << buf.getMsg(buf.begin(), pos);
        status_ = REQUEST_LINE_ERROR;
        return REQUEST_LINE_ERROR;
    }
    buf.retrieve(len + 1);
    /****************************解析url********************************/
    pos = buf.find(' ');
    if (!pos)
        return REQUEST_LINE;
    //move sematic
    //FIXME:keep-alive情况下，并发量一上来会导致解析出错，url变为GET
    //经排查发现是connection的buffer没有重置；后续发现buffer内部处理有问题，
    //简单来说，我把vector.size()和vector.capacity()搞混了
    url_ = buf.getMsg(buf.begin(), pos);
    //LOG_INFO << "URL:" << url_;
    if (url_ == "GET")
    {
        LOG_ERROR << "method:" << method_ << ", total message:" << buf.getAll();
    }
    len = buf.length(buf.begin(), pos);
    //move sematic
    path_ = url_.substr(url_.find('/'));
    buf.retrieve(len + 1);

    /******************************解析协议版本*****************************/
    pos = buf.find('/');
    if (!pos)
        return REQUEST_LINE;
    if (!buf.compare("HTTP", 4))
    {
        LOG_ERROR << "Unknown protocol:" << buf.getMsg(buf.begin(), pos);
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
    //LOG_INFO << "Parsing header.....";
    while (true)
    {
        auto pos = buf.findCRLF();
        if (!pos)
            return HEADERS;
        auto header(buf.getMsg(buf.begin(), pos));
        //LOG_INFO << "Header:" << header << ",length:" << header.length() << "\n";
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
    //TODO:解析请求正文
    status_ = DONE;
    return DONE;
}
void HttpRequest::resetRequest()
{
    headers_.clear();
    status_ = WAIT;
}
} // namespace net