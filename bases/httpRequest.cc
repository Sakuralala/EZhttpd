#include <algorithm>
#include <string.h>
#include "httpRequest.h"
#include "logger.h"
namespace net
{
bool HttpRequest::parse(bases::UserBuffer &buf)
{
    if (status_ == REQUEST_LINE)
        parseRequestLine(buf);
    else if (status_ == HEADERS)
        parseHeader(buf);
    else if (status_ == CONTENT)
        parseContent(buf);
    else
        LOG_ERROR << "Received reduant message.";
}
bool HttpRequest::parseRequestLine(bases::UserBuffer &buf)
{
    auto pos = buf.find(' ');
    if (!pos)
        return true;
    int len = buf.length(buf.begin(), pos);
    //方法
    if (len == 3)
    {
        if (buf.compare("GET", len))
            method_ = GET;
        else if (buf.compare("PUT", len))
            method_ = PUT;
        else
        {
            LOG_ERROR << "Unknown method.";
            return false;
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
            return false;
        }
    }
    else if (len == 6)
    {
        if (buf.compare("DELETE", len))
            method_ = DELETE;
        else
        {
            LOG_ERROR << "Unknown method.";
            return false;
        }
    }
    else
    {
        LOG_ERROR << "Unknown method.";
        return false;
    }
    buf.retrieve(len + 1);
    //url
    pos = buf.find(' ');
    if (!pos)
        return true;
    //move sematic
    url = buf.getMsg(buf.begin(), pos);

    buf.retrieve(len + 1);
    //version
    pos = buf.find('/');
    if (!pos)
        return true;
    if (!buf.compare("HTTP", 4))
    {
        LOG_ERROR << "Unknown protocol.";
        return false;
    }
    buf.retrieve(5);
    pos = buf.findCRLF();
    if (!pos)
        return true;
    //len = buf.length(buf.begin(), pos);
    if (buf.compare("1.1", 3))
        version_ = Ver_11;
    else if (buf.compare("1.0", 3))
        version_ = Ver_10;
    else
    {
        LOG_ERROR << "Unknown http version.";
        return false;
    }
    buf.retrieve(4);
    status_ = HEADERS;
    parseHeader(buf);
    return true;
}
bool HttpRequest::parseHeader(bases::UserBuffer &buf)
{
    auto pos = buf.findCRLF();
    if (!pos)
        return true;
    auto header(buf.getMsg(buf.begin(), pos));
    buf.retrieve(buf.length(buf.begin(), pos) + 1);
    auto dec= header.find(':');
    if (dec == std::string::npos)
    {
        LOG_ERROR<<"Invalid http header.";
        return false;
    }
    headers.emplace(header.substr(0,dec),header.substr(dec+1));
}
bool HttpRequest::parseContent(bases::UserBuffer &buf)
{
}
} // namespace net