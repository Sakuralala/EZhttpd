#include "httpUtils.h"
namespace net
{
const char *StatusString[7] = {"request line", "header",
                               "content", "done", "request line error", "header error", "content error"};
const char *VersionString[2] = {"HTTP/1.0", "HTTP/1.1"};
} // namespace net