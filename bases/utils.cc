#include <fcntl.h>
#include <sys/socket.h>
#include <string.h> //bzero
#include <netinet/in.h>
#include <errno.h> //strerror
#include "utils.h"
#include "../log/logger.h"
namespace bases
{
//  NOTE:多个连续的分隔符会被看作1个
std::vector<std::string> split(const std::string &str, const char ch)
{
    int lastPos = 0;
    while (lastPos < str.size() && str[lastPos] == ch)
        lastPos++;
    std::vector<std::string> ret;
    int curPos = lastPos;
    for (; curPos < str.size(); ++curPos)
    {
        if (str[curPos] == ch)
        {
            ret.push_back(str.substr(lastPos, curPos - lastPos));
            while (curPos < str.size() && str[curPos] == ch)
                curPos++;
            lastPos = curPos--;
        }
    }
    if (curPos > lastPos)
        ret.push_back(str.substr(lastPos, curPos - lastPos));
    return ret;
}

int createNonBlockingSocket()
{
    int socketFd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (socketFd == -1)
    {
        LOG_ERROR << "Create socket error:" << strerror(errno);
        return -1;
    }
    return socketFd;
}
int listen(int port)
{
    if (port <= 0)
    {
        LOG_ERROR << "Listen port is negtive!";
        return -1;
    }
    //创建socket->设置需要监听的ip:port->bind->listen
    struct sockaddr_in bindAddr;
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd == -1)
    {
        LOG_ERROR << "Create socket error:" << strerror(errno);
        return -1;
    }
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindAddr.sin_port = htons(port);
    //TODO:setsockopt之端口复用?
    //服务端建议开启端口复用，原因有以下几点:
    //1.可以在重启服务端时减少TIME_WAIT状态的等待;
    //2.可以在服务端具有多个网卡的情况下让不同的进程监听具有不同ip但是相同的port;
    //3.可以让服务端单进程...(同上)
    int optval = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        LOG_ERROR << "Reuse port socket error:" << strerror(errno);
        return -1;
    }
    if (bind(listenFd, (sockaddr *)&bindAddr, sizeof(bindAddr)) == -1)
    {
        LOG_ERROR << "Bind socket error:" << strerror(errno);
        return -1;
    }
    if (::listen(listenFd, 2048) == -1)
    {
        LOG_ERROR << "Listen socket error:" << strerror(errno);
        return -1;
    }
    return listenFd;
}
bool setNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1)
    {
        LOG_ERROR << "GETFL error:" << strerror(errno);
        return false;
    }
    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) == -1)
    {
        LOG_ERROR << "SETFL error:" << strerror(errno);
        return false;
    }
    return true;
}
} // namespace bases