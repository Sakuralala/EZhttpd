#include <algorithm>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "circularBuffer.h"
#include "../log/logger.h"

namespace bases
{
void CircularBuffer::resize()
{
    LOG_INFO << "Resized to " << buffer_.capacity() * 2 << ".";
    std::vector<char> buffer;
    buffer.reserve(buffer_.capacity() * 2);
    //虽然不太可能出现 还是要检测下当前buffer_是否为空
    if (isEmpty())
        head_ = tail_ = 0;
    if (head_ < tail_)
    {
        memcpy(&*buffer.begin(), begin(), tail_ - head_);
    }
    else
    {
        //把后一部分移到前面
        memcpy(&*buffer.begin(), begin(), buffer_.capacity() - head_);
        //把前一部分移到后面
        memcpy(&*buffer.begin() + buffer_.capacity() - head_, &*buffer_.begin(), head_);
    }

    tail_ = size();
    head_ = 0;
    buffer_.swap(buffer);
}
std::string CircularBuffer::getMsg(const char *beg, int len) const
{
    if (isEmpty() || len > size() || beg > end() || beg < begin() || beg + len > end())
        return "";
    int len1 = &*buffer_.begin() + buffer_.capacity() - beg;
    if (beg < end() || len <= len1)
        return std::string(beg, len);
    else
    {
        return std::string(beg, len1) + std::string(&*buffer_.begin(), len - len1);
    }
}
//FIXME:大小检查
//FIXED
std::string CircularBuffer::getMsg(const char *beg, const char *end) const
{
    if (beg < end)
        return std::string(beg, end - beg);
    else if (beg == end)
        return "";
    else
        return std::string(beg, &*buffer_.begin() + buffer_.capacity() - beg) +
               std::string(&*buffer_.begin(), end - &*buffer_.begin());
}
std::string CircularBuffer::getAll() const
{
    return getMsg(begin(), end());
}
bool CircularBuffer::compare(const char *s, int len) const
{
    if (isEmpty())
        return false;
    if (head_ < tail_)
        return !strncmp(s, begin(), len);
    else
    {
        int subLen = buffer_.capacity() - head_;
        return strncmp(s, begin(), subLen) & strncmp(s + subLen, &*buffer_.begin(), len - subLen);
    }
    //never go here.
    return true;
}

const char *CircularBuffer::find(char ch) const
{
    if (isEmpty())
        return nullptr;
    if (head_ < tail_)
        return std::find(begin(), end(), ch);
    else
    {
        auto pos = std::find(begin(), &*buffer_.begin() + buffer_.capacity(), ch);
        if (pos)
            return pos;
        pos = std::find(&*buffer_.begin(), end(), ch);
        if (pos)
            return pos;
    }
    return nullptr;
}
//TODO:这里存在将数据传递给上层时的数据分段问题，即一部分在尾部，一部分在头部，那么该如何处理:
//1.让上层进行处理，即需要先判断返回的crlf的位置和head_的大小关系来决定拷贝1次还是两次；
//2.内部处理，需要额外一次拷贝的开销；
const char *CircularBuffer::findCRLF() const
{
    if (isEmpty())
        return nullptr;
    //1.rw
    if (head_ < tail_)
    {
        auto ite = std::find(buffer_.begin() + head_, buffer_.begin() + tail_, '\r');
        if (ite < buffer_.begin() + tail_ - 1 && *(ite + 1) == '\n')
            return &*ite;
    }
    else //2.wr
    {
        auto ite = std::find(buffer_.begin() + head_,
                             buffer_.begin() + buffer_.capacity(), '\r');
        if (ite == buffer_.begin() + buffer_.capacity())
        {
            ite = std::find(buffer_.begin(), buffer_.begin() + tail_, '\r');
            //这种情况下消息被分割为不相邻的两部分了
            if (ite < buffer_.begin() + tail_ - 1 && *(ite + 1) == '\n')
                return &*ite;
        }
        else if (ite == buffer_.begin() + buffer_.capacity() - 1)
        {
            if (tail_ && *buffer_.begin() == '\n')
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
            tail_ = head_ = 0;
        if (tail_ >= head_)
        {
            //NOTE:新版的STL中vector的迭代器不再是普通指针，而是normal_iterator类
            LOG_DEBUG << "Try to read " << buffer_.capacity() - tail_ << " bytes.";
            n = ::read(fd, &*buffer_.begin() + tail_, buffer_.capacity() - tail_);
        }
        else
        {
            LOG_DEBUG << "Try to read " << head_ - tail_ << " bytes.";
            n = ::read(fd, &*buffer_.begin() + tail_, head_ - tail_);
        }
        if (n == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                LOG_DEBUG << "Read event is not ready in socket:" << fd;
            }
            else
            {
                if (errno == EPIPE)
                {
                    LOG_INFO << "Peer closed connection,socket:" << fd;
                }
                else
                {
                    LOG_ERROR << "Read event error:" << strerror(errno);
                }
                return -1;
            }
            break;
        }
        else if (n == 0) //对端关闭了写端或者直接关闭了连接，对于前者，说明请求发完了，那么正常处
        //理即可 对于后者，此时再写会出错,所以设置一个定时器，超时直接关闭连接
        {
            //存在这种情况，对端关闭连接导致读事件就绪，而当前读缓冲区为空导致前面将读写索引置0了,这里需要改回来
            if (!total && !head_ && !tail_)
                head_ = tail_ = -1;
            LOG_DEBUG << "Peer closed connection, socket:" << fd;
            break;
        }
        //n>0
        tail_ = (tail_ + n) % buffer_.capacity();
        total += n;
        LOG_DEBUG << "Read " << n << " bytes from socket:" << fd;
        //<< ",message:\n"
        //<< getAll();
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
                    LOG_DEBUG << "Write is not ready in socket:" << fd;
                }
                else
                {
                    if (errno == EPIPE)
                    {
                        LOG_DEBUG << "Write failed because peer closed connection,socket:" << fd;
                    }
                    else
                    {
                        LOG_ERROR << "Write error:" << strerror(errno) << " in socket:" << fd;
                    }
                    return -1;
                }
                break;
            }
            else if (n == 0)
            {
                LOG_ERROR << "Write returned 0 in socket:" << fd;
                return -1;
            }
            LOG_DEBUG << "Write " << n << " bytes to socket:" << fd; //<< ",message:\n"
                                                                     //<< msg;
            total += n;
            msg += n;
            len -= n;
        }
    }
    if (len) //把没发完的保存起来
    {
        //防止此时缓冲区为空
        if (isEmpty())
            head_ = tail_ = 0;
        if (remain() < len)
            resize();
        int tmp = buffer_.capacity() - tail_;
        //writeIndex<readIndex的情况是肯定没resize过的(因为resize会手动调整前后两段)
        //那么说明writeIndex后面的剩余部分肯定比要写的长
        //||后面的就是writeIndex>=readIndex的情况，如果剩余部分够，写一次就行了
        if (tail_ < head_ || len <= tmp)
        {
            memcpy(&*buffer_.begin() + tail_, msg, len);
        }
        else //tail_>head_&&len>=buffer_capacity()-tail_
        {
            memcpy(&*buffer_.begin() + tail_, msg, tmp);
            memcpy(&*buffer_.begin(), msg + tmp, len - tmp);
        }
        tail_ = (tail_ + len) % buffer_.capacity();
        //LOG_INFO << "Saved the remaining part.";
    }
    return total;
}

//写事件最终调用到的回调
int CircularBuffer::sendRemain(int fd)
{
    int n = 0, total = 0;
    while (!isEmpty())
    {
        if (head_ < tail_)
            n = ::write(fd, &*buffer_.begin() + head_, tail_ - head_);
        else
            n = ::write(fd, &*buffer_.begin() + head_, buffer_.capacity() - head_);

        if (n < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                LOG_DEBUG << "Write is not ready in socket:" << fd;
            }
            else
            {
                if (errno == EPIPE)
                {
                    LOG_INFO << "Write failed because peer closed connection,socket:" << fd;
                }
                else
                {
                    LOG_ERROR << "Write error:" << strerror(errno) << " in socket:" << fd;
                }
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
        LOG_DEBUG << "Write " << n << " bytes to socket:" << fd;
        head_ = (head_ + n) % buffer_.capacity();
        total += n;
        //发完清零 写缓冲区唯一一种会导致缓冲区为空的情况 即写缓冲区内部剩余的数据全写到内核缓冲
        //区中了
        if (head_ == tail_)
            head_ = tail_ = -1;
    }
    return total;
}

} // namespace bases