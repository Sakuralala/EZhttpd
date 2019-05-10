#ifndef __httputils_h
#define __httputils_h
/**
 *  http一些杂的定义； 
 * **/
namespace net
{
#define HTTP_OK 200
#define HTTP_BAD_REQUEST 400
#define HTTP_NOT_FOUND 404
#define HTTP_TIMEOUT 408

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
extern const char *VersionString[2];
enum ParseStatus
{
    WAIT,//在长连接的情况下，方便判断当前是否为新来的请求的起始部分，以便删除定时器
    REQUEST_METHOD,
    REQUEST_URL,
    PROTOCOL,
    HEADERS,
    CONTENT,
    DONE,
    REQUEST_METHOD_ERROR,
    REQUEST_URL_ERROR,
    PROTOCOL_ERROR,
    HEADERS_ERROR,
    CONTENT_ERROR
};
extern const char *StatusString[12];
} // namespace net
#endif