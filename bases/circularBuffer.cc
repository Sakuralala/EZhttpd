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
//TODO:这里存在将数据传递给上层时的数据分段问题，即一部分在尾部，一部分在头部，那么该如何处理:
//1.让上层进行处理，即需要先判断返回的crlf的位置和readIndex_的大小关系来决定拷贝1次还是两次；
//2.内部处理，需要额外一次拷贝的开销；
const char *CircularBuffer::findCRLF() const
{
    if (isEmpty())
        return nullptr;
    //1.rw
    if (readIndex_ < writeIndex_)
    {
        auto ite = std::find(buffer_.begin() + readIndex_, buffer_.begin() + writeIndex_, '\r');
        if (ite < buffer_.begin() + writeIndex_ - 1 && *(ite + 1) == '\n')
            return &*ite;
    }
    else //2.wr
    {
        auto ite = std::find(buffer_.begin() + readIndex_, buffer_.end(), '\r');
        if (ite == buffer_.end())
        {
            ite = std::find(buffer_.begin(), buffer_.begin() + writeIndex_, '\r');
            //这种情况下消息被分割为不相邻的两部分了
            if (ite < buffer_.begin() + writeIndex_ - 1 && *(ite + 1) == '\n')
                return &*ite;
        }
        else if (ite == buffer_.end() - 1)
        {
            if (writeIndex_ && *buffer_.begin() == '\n')
                return &*ite;
        }
        else
        {
            if (*(ite + 1) == '\n')
                return &*ite;
        }
    }
    return nullptr;
}
//仅支持非阻塞模式的读
//NOTE:读取数据肯定是等到读事件就绪才读的，但是写数据就不是了，只要把数据准备好了就可以写，
//至于什么时候才能把数据写到内核缓冲区内部，这是Buffer需要操心的事；
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
            {
                LOG_INFO << "Read event is not ready in socket:" << fd;
            }
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
//QUESTION:没发完的话如何设置写事件回调？
//ANSWER:上层发送数据和写事件回调是两回事，上层只要把数据准备好了就可以发数据，而何时能写
//数据到内核缓冲区由写事件回调完成；
int CircularBuffer::send(int fd, const char *msg, int len)
{
    //total指的是这次的数据发送了多少字节
    int n = 0, total = 0;
    //先把上次没发完的剩余的部分发过去
    if (sendRemain(fd) == -1)
        return -1;
    if (isEmpty()) //缓冲区内部的数据发完了,那么尝试先把这次的数据发送
    {
        //发送新添加的部分
        while (len)
        {
            n = ::write(fd, msg, len);
            if (n < 0)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    LOG_INFO << "Write is not ready in socket:" << fd;
                }
                else
                {
                    LOG_ERROR << "Write error:" << strerror(errno) << " in socket:" << fd;
                    return -1;
                }
                break;
            }
            else if (n == 0)
            {
                LOG_ERROR << "Write returned 0 in socket:" << fd;
                return -1;
            }
            LOG_INFO << "Write " << n << " bytes to socket:" << fd;
            total += n;
            msg += n;
            len -= n;
        }
    }
    if (len)
    {
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
    }
    return total;
}

//写事件最终调用到的回调
int CircularBuffer::sendRemain(int fd)
{
    int n = 0, total = 0;
    while (!isEmpty())
    {
        if (readIndex_ < writeIndex_)
            n = ::write(fd, &*buffer_.begin() + readIndex_, writeIndex_ - readIndex_);
        else
            n = ::write(fd, &*buffer_.begin() + readIndex_, buffer_.capacity() - readIndex_);

        if (n < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                LOG_INFO << "Write is not ready in socket:" << fd;
            }
            else
            {
                LOG_ERROR << "Write error:" << strerror(errno) << " in socket:" << fd;
                return -1;
            }
            break;
        }
        else if (n == 0)
        {
            LOG_ERROR << "Write returned 0 in socket:" << fd;
            return -1;
        }
        //n>0
        LOG_INFO << "Write " << n << " bytes to socket:" << fd;
        readIndex_ = (readIndex_ + n) % buffer_.capacity();
        total += n;
        //发完清零
        if (readIndex_ == writeIndex_)
            readIndex_ = writeIndex_ = -1;
    }
    return total;
}

} // namespace bases