#ifndef __buffer_h
#define __buffer_h
/***
 *记录日志时使用的底层缓冲区，大小在初始化时就确定了，主要功能就是将前端线程生成的消息复制到缓冲
 *区中；
 ***/
#include <string.h>
namespace bases
{
//日志线程的缓冲区队列的缓冲区大小
const int LargeBufferLength = 16 * 1024 * 1024;
//专门给logstream用的buffer，即一条日志消息的长度，这个想着怎么也不会这么长把 感觉最多100多字节就行了
const int SmallBufferLength = 256;
template <int Length>
class FixedLengthBuffer
{
  public:
    FixedLengthBuffer() : cur_(buffer_)
    {
        //memset(buffer_, 0, sizeof buffer_);
    }
    ~FixedLengthBuffer() {}
    bool append(const char *str, size_t len);
    void clear()
    {
        //memset(buffer_, 0, sizeof buffer_);
        cur_ = buffer_;
    }
    size_t available() const
    {
        return end() - cur_;
    }
    size_t used() const
    {
        return cur_ - buffer_;
    }
    size_t length() const
    {
        return cur_ - buffer_;
    }
    char *current() const
    {
        return cur_;
    }
    const char *begin() const
    {
        //buffer: char const[]
        return buffer_;
    }
    const char *end() const
    {
        return buffer_ + sizeof buffer_;
    }
    void add(size_t len)
    {
        cur_ += len;
    }

  private:
    char buffer_[Length];
    char *cur_;
};
template <int Length>
bool FixedLengthBuffer<Length>::append(const char *str, size_t len)
{
    if (available() < len)
        return false;
    memcpy(cur_, str, len);
    cur_ += len;
    return true;
}
} // namespace bases
#endif