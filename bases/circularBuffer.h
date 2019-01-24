#ifndef __circular_buffer_h
#define __circular_buffer_h
//#include <memory>
#include <vector>
/**
 * 用户态缓冲区，是一个环形的缓冲区，能够动态增长大小,以vector<char>为底层；
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
    if(isEmpty())
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
  int recv(int fd);
  int send(int fd, const char *msg, int len);

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