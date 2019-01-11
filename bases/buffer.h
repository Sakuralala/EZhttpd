#ifndef __buffer_h
#define __buffer_h
/***
 *记录日志时使用的底层缓冲区，大小在初始化时就确定了，主要功能就是将前端线程生成的消息复制到缓冲
 *区中；
 ***/
#include <string.h>
namespace bases
{
const int LargeBufferLength = 1024 * 1024 * 4;
const int SmallBufferLength = 1024 * 4;
template <int Length>
class FixedLengthBuffer
{
  public:
    FixedLengthBuffer() : cur_(buffer_) {}
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
        return sizeof buffer_;
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
    void add(size_t len)
    {
        cur_ += len;
    }

  private:
    char buffer_[Length];
    char *cur_;
    const char *end() const
    {
        return buffer_ + sizeof buffer_;
    }
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