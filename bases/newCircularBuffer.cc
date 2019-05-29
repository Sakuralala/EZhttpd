#include <algorithm>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "newCircularBuffer.h"
#include "../log/logger.h"

namespace bases
{
CharCircularBuffer::iterator CharCircularBuffer::findCRLF()
{
    if (empty())
        return end();
    //1.rw
    if (head_ < tail_)
    {
        auto ite = std::find(begin(), end(), '\r');
        if (ite < end() - 1 && *(ite + 1) == '\n')
            return ite;
    }
    else //2.wr
    {
        auto ite = std::find(begin(), actualEnd(), '\r');
        if (ite == actualEnd())
        {
            ite = std::find(actualBegin(), end(), '\r');
            //这种情况下消息被分割为不相邻的两部分了
            if (ite < end() - 1 && *(ite + 1) == '\n')
                return ite;
        }
        else if (ite == actualEnd() - 1)
        {
            if (tail_ && *actualBegin() == '\n')
                return ite;
        }
        else
        {
            if (*(ite + 1) == '\n')
                return ite;
        }
    }
    return end();
}
CharCircularBuffer::iterator CharCircularBuffer::find(char ch)
{
    if (empty())
        return end();
    if (head_ < tail_)
        return std::find(begin(), end(), ch);
    else
    {
        auto pos = std::find(begin(), actualEnd(), ch);
        if (pos != end())
            return pos;
        pos = std::find(actualBegin(), end(), ch);
        if (pos != end())
            return pos;
    }
    return end();
}

std::string CharCircularBuffer::getMsg(iterator first, iterator last)
{
    if (first < last)
        return std::string(base_.get() + first.pos_, last - first);
    else if (first == last)
        return "";
    else
        return std::string(base_.get() + first.pos_, capacity_ - first.pos_) + std::string(base_.get(), last.pos_);
}

std::string CharCircularBuffer::getMsg(iterator first, size_t len)
{
    return getMsg(first, first + len);
}
std::string CharCircularBuffer::getAll()
{
    return getMsg(begin(), end());
}
int CharCircularBuffer::recv(int fd)
{
    LOG_DEBUG << "New data ready.";
    int n = 0, total = 0;
    while (true)
    {
        if (full())
            resize();
        if (!reversed())
        {
            LOG_DEBUG << "Try to read " << capacity_ - tail_ << " bytes.";
            n = ::read(fd, base_.get() + tail_, capacity_ - tail_);
        }
        else
        {
            LOG_DEBUG << "Try to read " << head_ - tail_ << " bytes.";
            n = ::read(fd, base_.get() + tail_, head_ - tail_);
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
        //FIXME: maybe EOF, not the peer closed the connection.
        //FIXED:distinguish with total.
        {
            if (!total)
            {
                LOG_DEBUG << "Peer closed connection, socket:" << fd;
            }
            break;
        }
        //n>0
        tail_ = (tail_ + n) % capacity_;
        dataSize_ += n;
        total += n;
        LOG_DEBUG << "Read " << n << " bytes from socket:" << fd;
        //<< ",message:\n"
        //<< getAll();
    }
    return total;
}
int CharCircularBuffer::send(int fd, const char *msg, int len)
{
    int n = 0, total = 0;
    //先把上次没发完的剩余的部分发过去
    if (sendRemain(fd) == -1)
        return -1;
    if (empty()) //缓冲区内部的数据发完了,那么尝试先把这次的数据发送
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
        if (remain() < len)
            resize();
        int tmp = capacity_ - tail_;
        //tail_<head_的情况是肯定没resize过的(因为resize会手动调整前后两段)
        //那么说明tail_后面的剩余部分肯定比要写的长
        //||后面的就是tail_>=head_的情况，如果剩余部分够，写一次就行了
        if (tail_ < head_ || len <= tmp)
        {
            memcpy(base_.get() + tail_, msg, len);
        }
        else //tail_>head_&&len>=capacity_-tail_
        {
            memcpy(base_.get() + tail_, msg, tmp);
            memcpy(base_.get(), msg + tmp, len - tmp);
        }
        tail_ = (tail_ + len) % capacity_;
        dataSize_ += len;
        LOG_DEBUG << "Saved the remaining part,size:" << len;
    }
    else
    {
        LOG_INFO << "Write completely.";
    }
    return total;
}

//写事件最终调用到的回调
int CharCircularBuffer::sendRemain(int fd)
{
    int n = 0, total = 0;
    while (!empty())
    {
        if (!reversed())
            n = ::write(fd, base_.get() + head_, tail_ - head_);
        else
            n = ::write(fd, base_.get() + head_, capacity_ - head_);

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
        head_ = (head_ + n) % capacity_;
        dataSize_ -= n;
        total += n;
    }
    if (empty())
        head_ = tail_ = 0;
    return total;
}
} // namespace bases