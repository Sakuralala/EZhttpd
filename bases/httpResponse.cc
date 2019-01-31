#include <fstream>
#include "httpResponse.h"
#include "timer.h"
#include "connection.h"
#include "logger.h"
namespace net
{
std::unordered_map<int, std::string> HttpResponse::responseCodes_ =
    {
        {200, "OK"},
        {400, "Bad request"},
        {404, "Resource not found"},
        {408, "Request timeout"}};
//TODO:改掉下面的硬编码
const std::string HttpResponse::basePath_ = "/home/oldhen/httptest/";

HttpResponse::HttpResponse(HttpVersion ver, const std::string &path) : responseCode_(HTTP_OK),
                                                                       responseLine_(VersionString[ver])
{
    constructResponse(path);
}
//仅限404 408
HttpResponse::HttpResponse(HttpVersion ver, int responseCode) : responseCode_(responseCode),
                                                                responseLine_(VersionString[ver])
{
    if (responseCode == HTTP_BAD_REQUEST)
        badRequest();
    else if (responseCode == HTTP_NOT_FOUND)
        notFound();
    else if (responseCode == HTTP_OK)
        constructResponse();
    else
        LOG_ERROR << "Unsupported response code:" << responseCode;
}
HttpResponse::~HttpResponse()
{
}
void HttpResponse::badRequest()
{
    constructResponse("bad_request.html");
}
void HttpResponse::notFound()
{
    constructResponse("not_found.html");
}
void HttpResponse::constructResponse(const std::string &path)
{
    std::ifstream inFile(basePath_ + path);
    if (!inFile)
    {
        responseCode_ = HTTP_NOT_FOUND;
        inFile.clear();
        inFile.open(basePath_ + "not_found.html");
    }
    if (inFile)
    {
        std::string line;
        //响应正文
        while (std::getline(inFile, line))
            content_ += line;
    }
    //响应行
    responseLine_ += ' ';
    responseLine_ += std::to_string(responseCode_);
    responseLine_ += ' ';
    responseLine_ += responseCodes_[responseCode_];
    //响应头
    headers_.emplace("Server: EZHttpd\r\n");
    headers_.emplace("Date: " + event::Timer(0).format() + "\r\n");
    headers_.emplace("Content-Length: " + std::to_string(content_.size()) + "\r\n");
    headers_.emplace("Content-Type: text/html\r\n");
}
void HttpResponse::sendResponse(ConnectionPtr &conn) const
{
    conn->send(responseLine_);
    for (auto &header : headers_)
        conn->send(header);

    if (content_.size())
        conn->send(content_);
    //conn->send("\r\n");
}
} // namespace net