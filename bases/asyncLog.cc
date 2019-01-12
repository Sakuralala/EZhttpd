#include "asyncLog.h"
#include "logFile.h"

namespace bases
{

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
        //current_->append("Change to next buffer.",24);
        current_->append(message, length);
    }
    //current_->append(message, length);
    cond_.notify();
    return true;
}

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
                cond_.timedWait(DefaultWriteSeconds);
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
            /*
            {
                MutexGuard mg(mutex_);
                empty_.emplace_back(std::move(buffer));
                //等到全写完了再整体清理 pop_front会造成大量的整体移动
                //fullCopy.pop_front();
            }
            */
        }
        //TODO:比较两种方式的效率
        //全写完了再一起放回去
        {
            MutexGuard mg(mutex_);
            for (auto &buffer : fullCopy)
                empty_.emplace_back(std::move(buffer));
            fullCopy.clear();
        }
        lf.flush();
    }
    //TODO:善后处理 把剩余的日志全部写入
    lf.flush();
}
} // namespace bases