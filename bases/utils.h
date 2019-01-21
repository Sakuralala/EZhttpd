#ifndef __utils_h
#define __utils_h

/***
 *一些杂的底层操作，主要是一些socket操作；   
 * **/
namespace bases
{
int listen(int port);
bool setNonBlocking(int fd);
} // namespace bases
#endif 