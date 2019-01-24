#include <algorithm>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "circularBuffer.h"
#include "logger.h"

namespace bases
{
void CircularBuffer::resize()
{
    buffer_.reserve(buffer_.capacity() * 2);
    std::copy(buffer_.begin(), buffer_.begin() + writeIndex_, buffer_.begin() + buffer_.capacity() / 2);
    writeIndex_ += buffer_.capacity() / 2;
}
//仅支持非阻塞模式的读
int CircularBuffer::recv(int fd)
{
    int n = 0, total = 0;
    while (true)
    {
        if (isFull())
            resize();
        if (isEmpty())
            writeIndex_ = readIndex_ = 0;
        if (writeIndex_ >= readIndex_)
        {
            //NOTE:新版的STL中vector的迭代器不再是普通指针，而是normal_iterator类
            n = ::read(fd, &*buffer_.begin() + writeIndex_, buffer_.capacity() - writeIndex_);
        }
        else
        {
            n = ::read(fd, &*buffer_.begin() + writeIndex_, readIndex_ - writeIndex_);
        }
        if (n == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                LOG_INFO << "Read event is not ready in socket:" << fd;
            else
            {
                LOG_ERROR << "Read event error:" << strerror(errno);
                return -1;
            }
            break;
        }
        else if (n == 0)
        {
            LOG_INFO << "Read eof from socket:" << fd;
            break;
        }
        //n>0
        LOG_INFO << "Read " << n << " bytes from socket:" << fd;
        writeIndex_ = (writeIndex_ + n) % buffer_.capacity();
        total += n;
    }
    return total;
}
int CircularBuffer::send(int fd, const char *msg, int len)
{
    //total指的是这次的数据发送了多少字节
    int n = 0, total = 0;
    //先把上次没发完的剩余的部分发过去
    while (!isEmpty())
    {
        if (readIndex_ < writeIndex_)
            n = ::write(fd, &*buffer_.begin() + readIndex_, writeIndex_ - readIndex_);
        else
            n = ::write(fd, &*buffer_.begin() + readIndex_, buffer_.capacity() - readIndex_);

        if (n < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                LOG_INFO << "Write is not ready.";
            else
            {
                LOG_ERROR << "Write error:" << strerror(errno);
                return -1;
            }
            break;
        }
        else if (n == 0)
        {
            LOG_ERROR << "Write returned 0.";
            return -1;
        }
        //n>0
        readIndex_ = (readIndex_ + n) % buffer_.capacity();
        //发完清零
        if (readIndex_ == writeIndex_)
            readIndex_ = writeIndex_ = -1;
    }
    //发送新添加的部分
    while (len)
    {
        n = ::write(fd, msg, len);
        if (n < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                LOG_INFO << "Write is not ready.";
            else
            {
                LOG_ERROR << "Write error:" << strerror(errno);
                return -1;
            }
            break;
        }
        else if (n == 0)
        {
            LOG_ERROR << "Write returned 0.";
            return -1;
        }
        total += n;
        msg += n;
        len -= n;
    }
    //把没发完的保存起来
    if (remain() < len)
        resize();

    int tmp = buffer_.capacity() - writeIndex_;
    if (writeIndex_ < readIndex_ || len <= tmp)
    {
        memcpy(&*buffer_.begin() + writeIndex_, msg, len);
    }
    else //writeIndex_>readIndex_&&len>=buffer_capacity()-writeIndex_
    {
        memcpy(&*buffer_.begin() + writeIndex_, msg, tmp);
        memcpy(&*buffer_.begin(), msg + tmp, len - tmp);
    }
    writeIndex_ = (writeIndex_ + len) % buffer_.capacity();

    return total;
}
} // namespace bases