#ifndef __circular_buffer_h
#define __circular_buffer_h
//#include <memory>
#include <vector>
#include <string>
/**
 * 用户态缓冲区，是一个环形的缓冲区，能够动态增长大小,以vector<char>为底层；采用环形缓冲区
 * 的优势就是能够更好地利用内存空间(主要是前面起始部分在运行过程中留下的空间)，但是缺点就是代码
 * 编写时需要考虑数据在首尾两端的情况，需要额外的处理过程,如果在内部进行处理，那么就有额外拷贝
 * 产生的开销，如果让上层自行进行处理，代码就会不好看。  
 * **/
namespace bases
{
class CircularBuffer
{
private:
  static const int InitialSize = 8192;

public:
  CircularBuffer() : buffer_(InitialSize, 0), readIndex_(-1), writeIndex_(-1) {}
  ~CircularBuffer() = default;
  int size() const
  {
    if (isEmpty())
      return 0;
    return (writeIndex_ - readIndex_ + buffer_.capacity()) % buffer_.capacity();
  }
  int remain() const
  {
    return buffer_.capacity() - size();
  }
  int capacity() const
  {
    return buffer_.capacity();
  }
  //读事件最终调用的回调
  int recv(int fd);
  //上层调用者写
  int send(int fd, const char *msg, int len);
  //写事件最终调用的回调
  int sendRemain(int fd);
  const char *begin() const
  {
    if (isEmpty())
      return nullptr;
    return &*buffer_.begin() + readIndex_;
  }
  const char *end() const
  {
    if (isEmpty())
      return nullptr;
    return &*buffer_.begin() + writeIndex_;
  }
  //方便解析http请求头
  const char *findCRLF() const;
  const char *find(char ch) const;
  int length(const char *beg, const char *end)
  {
    if (isEmpty())
      return -1;
    return (end - beg + buffer_.capacity()) % buffer_.capacity();
  }
  bool compare(const char *s, int len) const;
  std::string getMsg(const char *beg, const char *end) const;
  //readIndex_前进len字节
  bool retrieve(int len)
  {
    if (len > size())
      return false;
    readIndex_ = (readIndex_ + len) % buffer_.capacity();
    if (readIndex_ >= writeIndex_)
      readIndex_ = writeIndex_ = -1;
    return true;
  }

private:
  std::vector<char> buffer_;
  //readIndex==writeIndex时如何判断当前缓冲区是满还是空?
  //为空时置均为-1
  int readIndex_;
  int writeIndex_;
  bool isFull() const
  {
    return readIndex_ != -1 && readIndex_ == writeIndex_;
  }
  bool isEmpty() const
  {
    return readIndex_ == writeIndex_;
  }
  void resize();
};
typedef CircularBuffer UserBuffer;
} // namespace bases
#endif