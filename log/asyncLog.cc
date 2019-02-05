#include "logger.h"
#include "asyncLog.h"
#include "logFile.h"

namespace bases
{
std::string LogPath = "/home/oldhen/logfile/";
AsyncLog::AsyncLog(const std::string &pathname) : pathName_(pathname), thread_(std::bind(&AsyncLog::writeToFile, this), "Log thread"), latch_(1), mutex_(), cond_(mutex_), looping_(false), current_(new LogBuffer), empty_(MaxNumber - 1)
{
    full_.reserve(MaxNumber);
    for (int i = 0; i < empty_.size(); ++i)
        //empty_.emplace_back(new LogBuffer);
        empty_[i] = std::unique_ptr<LogBuffer>(new LogBuffer);
}
AsyncLog::~AsyncLog()
{
    stop();
}
void AsyncLog::run()
{
    thread_.run();
    //等待线程进入后端写函数
    latch_.wait();
}
void AsyncLog::stop()
{
    looping_ = false;
    thread_.join();
}
bool AsyncLog::append(const char *message, size_t length)
{
    //看起来临界区很大 其实并不耗时  因为除了copy消息其他都是指针交换或移动或赋值操作
    MutexGuard mg(mutex_);
    //当前缓冲区用完了
    if (current_->append(message, length) == false)
    {
        //总共就16块 用完了就没了
        //muduo中的处理方式是直接扔了后面的部分
        //这里的处理方式是到达阈值了直接不让写了
        if (!empty_.empty())
        {
            //放这里是为了保证current不为空
            full_.push_back(std::move(current_));
            current_ = std::move(empty_.back());
            empty_.pop_back();
        }
        else
            return false;
        current_->append(message, length);
    }
    cond_.notify();
    return true;
}

//FIXME:某些时候会出现没写入文件的情况
//FIXED:似乎修复了，在后端写的最后退出循环时再把所有的buffer写一遍
//FIXME:修复有些日志重复写的问题;
//FIXED:按键盘的时候不小心按错大括号了导致逻辑不对了。。。。不是bug，改回原有的逻辑没问题了；
void AsyncLog::writeToFile()
{
    looping_ = true;
    BufferVector fullCopy;
    fullCopy.reserve(MaxNumber);
    LogFile lf(pathName_);
    latch_.countdown();
    while (looping_)
    {
        {
            MutexGuard mg(mutex_);
            if (full_.empty())
            {
                cond_.timedWait(DefaultWriteSeconds);
            }
            //只有在还有剩余buffer的时候才把current也拿过来写入文件
            if (!empty_.empty())
            {
                //std::unique_ptr调用了移动语义的构造或赋值函数后被移动的会被置为空 可放心使用
                full_.push_back(std::move(current_));
                current_ = std::move(empty_.back());
                empty_.pop_back();
            }
            fullCopy.swap(full_);
        }
        for (auto &buffer : fullCopy)
        {
            //写入文件
            lf.append(buffer->begin(), buffer->length());
            //每写完一块就把空闲的放回去
            //由于是多生产者单消费者问题，所以这里可能会长时间等锁反而得不偿失
            //结果:5.99user 4.35system 0:03.65elapsed 283%CPU (0avgtext+0avgdata 6140maxresident)k0inputs+846976outputs (0major+861minor)pagefaults 0swaps
            /*
            {
                MutexGuard mg(mutex_);
                buffer->clear();
                empty_.emplace_back(std::move(buffer));
            }
            */
        }
        //TODO:比较两种方式的效率
        //全写完了再一起放回去
        //测试情景:4个生产者线程，man open的每个单词单独写一行；重复250次(上面的sleep被注释了)
        //结果:6.08user 3.93system 0:03.63elapsed 275%CPU (0avgtext+0avgdata 6920maxresident)k0inputs+846976outputs (0major+1064minor)pagefaults 0swaps
        //似乎是差不多；
        //也达到了muduo中所说的100MB/s的要求；
        {
            MutexGuard mg(mutex_);
            for (auto &buffer : fullCopy)
            {
                buffer->clear();
                empty_.emplace_back(std::move(buffer));
            }
        }
        //如果把clear放在临界区内，速度竟然会慢5s之多！！！(按行输出man open的每个单词)
        fullCopy.clear();
        lf.flush();
    }

    //善后处理 把剩余的日志全部写入
    {
        MutexGuard mg(mutex_);
        for (auto &buffer : full_)
        {
            lf.append(buffer->begin(), buffer->length());
        }
        lf.append(current_->begin(), current_->length());
    }
    lf.flush();
}
} // namespace bases