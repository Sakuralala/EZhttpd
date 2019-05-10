#include "httpUtils.h"
namespace net
{
const char *StatusString[12] = {"wait","request method","request url","protocol", "headers",
                               "content", "done", "request method error","request url error","protocol error", "headers error", "content error"};
const char *VersionString[2] = {"HTTP/1.0", "HTTP/1.1"};
} // namespace net