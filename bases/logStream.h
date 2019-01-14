#ifndef __logstream_h
#define __logstream_h

/**
 *流式日志风格，主要是为了简化前端生成日志消息； 
 * **/
#include <string>
#include "buffer.h"

namespace bases
{
class LogStream
{
  public:
    typedef FixedLengthBuffer<SmallBufferLength> SmallBuffer;
    LogStream() {}
    ~LogStream()=default;
    SmallBuffer &getBuffer()
    {
        return buffer_;
    }
    LogStream &operator<<(bool val)
    {
        buffer_.append(val ? "1" : "0", 1);
        return *this;
    }
    LogStream &operator<<(char ch)
    {
        buffer_.append(&ch, 1);
        return *this;
    }
    /*不允许这种重载
    LogStream &operator<<(unsigned char ch)
    {
        //若大于char的最大值就不对了 
        //return operator<<(static_cast<char>(ch));

    }
    */
    LogStream &operator<<(char *str)
    {
        if (str)
        {
            buffer_.append(str, strlen(str));
        }
        return *this;
    }
    LogStream &operator<<(const char *str)
    {
        return operator<<(const_cast<char *>(str));
    }
    //不允许unsigned char *

    LogStream &operator<<(const std::string &s)
    {
        buffer_.append(s.c_str(), s.size());
        return *this;
    }
    /********************************数字相关****************************************/
    LogStream &operator<<(short val);
    LogStream &operator<<(unsigned short val);
    LogStream &operator<<(int val);
    LogStream &operator<<(unsigned int val);
    LogStream &operator<<(long val);
    LogStream &operator<<(unsigned long val);
    LogStream &operator<<(long long val);
    LogStream &operator<<(unsigned long long val);
    LogStream &operator<<(float val);
    LogStream &operator<<(double val);

  private:
    SmallBuffer buffer_;
    //用于快速将数字转换为字符串
    template <typename T>
    void formatInterger(T val);
};
//最简单的实现流式Log的方式，当然，为了提供不同的log级别，我们需要再封装一层
//#define Log LogStream()
} // namespace bases
#endif // !__logstream_h
