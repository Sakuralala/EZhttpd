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
    REQUEST_LINE,
    HEADERS,
    CONTENT,
    DONE,
    REQUEST_LINE_ERROR,
    HEADERS_ERROR,
    CONTENT_ERROR
};
extern const char *StatusString[7];
} // namespace net
#endif