//#include <algorithm>
#include <stdio.h> //for snprintf
#include "logStream.h"
//#include "asyncLog.h"
//#include "singleton.h"
namespace bases
{
const int MaxNumberLength = 32;
const char digits[] = "9876543210123456789";
const char *zero = digits + 9;
//一个后端线程就行
//AsyncLog *logThreadPtr = nullptr;

//快速将数字转换为字符串的算法,copy from muduo.
template <typename T>
size_t convert(char *buf, T val)
{
    char *begin = buf;
    while (val)
    {
        *buf++ = zero[val % 10];
        val /= 10;
    }
    if (val < 0)
        *buf++ = '-';
    //TODO:为啥要加
    *buf = '\0';
    //std::reverse(begin, buf);
    size_t len = buf - begin;
    buf--;
    while (begin < buf)
    {
        //整数  没问题
        *begin ^= *buf;
        *buf ^= *begin;
        *begin ^= *buf;
        begin++, buf--;
    }
    return len;
}
//在析构时将消息输出到日志
/*
LogStream::~LogStream()
{
    *this << "\n";
    //多线程环境下，保证初始化函数只被调用一次  pthread_once 或者c++11的call_once
    Singleton<AsyncLog>::getInstance()->append(buffer_.begin(),buffer_.length());
}
*/

template <typename T>
void LogStream::formatInterger(T val)
{
    if (buffer_.available() >= MaxNumberLength)
    {
        size_t len = convert(buffer_.current(), val);
        buffer_.add(len);
    }
}

LogStream &LogStream::operator<<(short val)
{
    return *this << static_cast<int>(val);
}
LogStream &LogStream::operator<<(unsigned short val)
{
    return *this << static_cast<unsigned int>(val);
}
LogStream &LogStream::operator<<(int val)
{
    formatInterger(val);
    return *this;
}
LogStream &LogStream::operator<<(unsigned int val)
{
    formatInterger(val);
    return *this;
}
LogStream &LogStream::operator<<(long val)
{
    formatInterger(val);
    return *this;
}
LogStream &LogStream::operator<<(unsigned long val)
{
    formatInterger(val);
    return *this;
}
LogStream &LogStream::operator<<(long long val)
{
    formatInterger(val);
    return *this;
}
LogStream &LogStream::operator<<(unsigned long long val)
{
    formatInterger(val);
    return *this;
}
LogStream &LogStream::operator<<(float val)
{
    return *this << static_cast<double>(val);
}
//什么grisus3 打扰了
LogStream &LogStream::operator<<(double val)
{
    if (buffer_.available() >= MaxNumberLength)
    {
        int len = snprintf(buffer_.current(), MaxNumberLength, "%.12g", val);
        buffer_.add(len);
    }
    return *this;
}
} // namespace bases
