#ifndef __httpresponse_h
#define __httpresponse_h
/**
 * http响应； 
 * **/
#include <unordered_map>
//#include <unordered_set>
#include <string>
#include <memory>
#include "httpUtils.h"
namespace net
{
class Connection;
typedef std::shared_ptr<Connection> ConnectionPtr;
class HttpResponse
{
public:
  HttpResponse(HttpVersion ver, const std::string &path = "not_found.html");
  HttpResponse(HttpVersion ver, int responseCode = HTTP_OK);
  ~HttpResponse();
  int sendResponse(const ConnectionPtr &conn) const;

private:
  static std::unordered_map<std::string, std::string> mime_;
  int responseCode_;
  //响应行
  std::string responseLine_;
  //响应头
  std::unordered_map<std::string,std::string> headers_;
  //响应正文
  std::string content_;
  //响应码
  static std::unordered_map<int, std::string> responseCodes_;
  static const std::string basePath_;
  void badRequest();
  void notFound();
  void constructResponse(const std::string &path = "ok.html");
};
} // namespace net

#endif // !__httpresponse_h