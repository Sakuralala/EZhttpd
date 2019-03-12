#include <string.h>
#include <assert.h>
#include <errno.h>
#include "fileUtil.h"
namespace bases
{
AppendFile::AppendFile(const std::string &pathname):file_(fopen(pathname.c_str(), "ae")), writeBytes(0)
{
    //file_ = fopen(pathname.c_str(),"ae");
    if(!file_)
        fprintf(stderr, "Create file stream failed:%s.,the file name is:%s.\n", strerror(errno),pathname.c_str());
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
    writeBytes = 0;
}
*/
size_t AppendFile::append(const char *msg, size_t len)
{
    size_t cur = 0;
    while (len)
    {
        size_t writed = fwrite(msg, 1, len, file_);
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
    writeBytes += cur;
    return cur;
}
void AppendFile::flush()
{
    fflush(file_);
}
} // namespace bases