#include <stdio.h> //for stdout
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include "fileUtil.h"
#include "../log/logger.h"
namespace bases
{
AppendFile::AppendFile(const std::string &pathname) : file_(fopen(pathname.c_str(), "abe")), name_(pathname), writeBytes_(0)
{
    //file_ = fopen(pathname.c_str(),"ae");
    if (!file_)
    {
        fprintf(stderr, "Open file %s error:%s.\n", pathname.c_str(), strerror(errno));
    }
    //设置流的缓冲区
    //setbuffer(file_, buffer_, sizeof buffer_);
    //TODO:比较是否使用这个缓冲区的效率
    //DONE:基本一致
    setbuffer(file_, nullptr, 0);
}
AppendFile::~AppendFile()
{
    if (file_)
        fclose(file_);
}
/*
void AppendFile::reset(const std::string &pathname)
{
    if (file_)
        fclose(file_);
    file_ = fopen(pathname.c_str(), "w");
    writeBytes_ = 0;
}
*/
size_t AppendFile::append(const char *msg, size_t len)
{
    size_t cur = 0;
    while (len)
    {
        size_t writed = fwrite(msg, 1, len, file_);
#ifdef LOG_STDOUT
        //输出到标准输出
        fwrite(msg, 1, len, stdout);
#endif
        if (!writed)
        {
            int err = ferror(file_);
            if (err)
            {
                fprintf(stderr, "Write to log file failed,error:%s.\n", strerror(err));
            }
            break;
        }
        len -= writed;
        cur += writed;
    }
    writeBytes_ += cur;
    return cur;
}
void AppendFile::flush()
{
    fflush(file_);
}
ReadFile::ReadFile(const std::string &name) : file_(::fopen(name.c_str(), "rbe")), name_(name)
{
    if (!file_)
    {
        fprintf(stderr, "Open file %s error:%s.\n", name.c_str(), strerror(errno));
    }
    //setbuffer(file_, buffer_, 0);
}
//TODO:目前需要用户态缓冲区的两次拷贝，需要减小到一次；
bool ReadFile::readtoString(std::string &result)
{
    struct stat statBuf;
    stat(name_.c_str(), &statBuf);
    result.reserve(statBuf.st_size);
    size_t cur = 0;
    size_t len = std::min(static_cast<size_t>(statBuf.st_size), static_cast<size_t>(bufferedSize));
    while (result.size() != statBuf.st_size)
    {
        size_t readSize = fread(buffer_, 1, len, file_);
        if (!readSize)
        {
            int err = ferror(file_);
            if (err)
            {
                fprintf(stderr, "Read file failed,error:%s.\n", strerror(err));
                return false;
            }
            break;
        }
        len -= readSize;
        cur += readSize;
        result.append(buffer_, readSize);
    }
    readBytes_ += cur;
    return true;
}
} // namespace bases