#ifndef __fileutil_h
#define __fileutil_h
/***
 *一些底层文件操作的封装;主要是读写文件流;
 * **/
#include <string>
#include <stdio.h>
#ifndef LOG_STDOUT
#define LOG_STDOUT
#endif // !LOG_STDOUT
//#undef LOG_STDOUT
namespace bases
{
//写文件的封装
class AppendFile
{
public:
  AppendFile(const std::string &pathname);
  ~AppendFile();
  off_t getCurrentSize() const
  {
    return writeBytes_;
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
  std::string name_;
  size_t writeBytes_;
};
//TODO:readfile
class ReadFile
{
  public:
  ReadFile(const std::string &name);
  ~ReadFile()
  {
    if (file_)
      ::fclose(file_);
  }
  bool readtoString(std::string &result);

private:
  FILE *file_;
  std::string name_;
  //缓冲的大小
  static const int bufferedSize = 64 * 1024;
  //block buffered
  char buffer_[bufferedSize];
  size_t readBytes_;
};
} // namespace bases
#endif // !1