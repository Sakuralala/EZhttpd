#ifndef __fileutil_h
#define __fileutil_h
/***
 *一些底层文件操作的封装;主要是读写文件流;
 * **/
#include <string>
#include <stdio.h>
namespace bases
{
//写文件的封装  使用了64mb的缓冲区
class AppendFile
{
  public:
    AppendFile(const std::string &pathname);
    ~AppendFile();
    off_t getCurrentSize() const
    {
        return writeBytes;
    }
    //void reset(const std::string &pathname);
    size_t append(const char *msg, size_t len);
    void flush();

  private:
  //缓冲的大小
    static const int bufferedSize = 64 * 1024 * 1024;
    //block buffered
    char buffer_[bufferedSize];
    FILE *file_;
    size_t writeBytes;
};
//TODO:readfile
class ReadFile
{
};
} // namespace bases
#endif // !1