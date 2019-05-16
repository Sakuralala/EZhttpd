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
  static const int InitialSize = 4096;

public:
  CircularBuffer() : head_(-1), tail_(-1)
  {
    buffer_.reserve(InitialSize);
  }
  ~CircularBuffer() = default;
  int size() const
  {
    if (isEmpty())
      return 0;
    return (tail_ + buffer_.capacity() - head_) % buffer_.capacity();
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
  bool isFull() const
  {
    return head_ != -1 && head_ == tail_;
  }
  bool isEmpty() const
  {
    return head_ == -1;
  }
  const char *begin() const
  {
    if (isEmpty())
      return nullptr;
    return &*buffer_.begin() + head_;
  }
  const char *end() const
  {
    if (isEmpty())
      return nullptr;
    return &*buffer_.begin() + tail_;
  }
  //TODO:以下均为为了方便解析http请求而写的，看着很膈应
  //找某个元素、计算两个位置间的距离、比较某段和一个字符串、取得某段数据
  //
  const char *findCRLF() const;
  const char *find(char ch) const;
  int length(const char *beg, const char *end)
  {
    if (isEmpty())
      return 0;
    return (end + buffer_.capacity() - beg) % buffer_.capacity();
  }
  bool compare(const char *s, int len) const;
  std::string getMsg(const char *beg, const char *end) const;
  std::string getMsg(const char *beg, int len) const;
  std::string getAll() const;
  //head_前进len字节
  bool retrieve(int len)
  {
    if (len > size())
      return false;
    head_ = (head_ + len) % buffer_.capacity();
    //读缓冲区唯一一种会导致缓冲区变为空的情况 即读缓冲区中的数据已经被解析完了
    if (head_ >= tail_)
      head_ = tail_ = -1;
    return true;
  }
  void reset()
  {
    head_ = tail_ = -1;
  }

private:
  std::vector<char> buffer_;
  //head==tail时如何判断当前缓冲区是满还是空?
  //为空时置均为-1
  int head_;
  int tail_;
  void resize();
};
} // namespace bases
#endif