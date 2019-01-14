#include <iostream>
#include "IOThreadPool.h"
#include "IOThread.h"
#include "eventLoop.h"
#include "logger.h"
namespace event
{
IOThreadPool::IOThreadPool() : tid_(bases::currentThreadID()),next_(0),started_(false){}
IOThreadPool::~IOThreadPool() = default;
void IOThreadPool::start(uint32_t num)
{
    IOThreads_.reserve(num);
    loops_.reserve(num);
    while (num--)
    {
        IOThreads_.emplace_back(new IOThread());
        IOThreads_.back()->run();
        loops_.push_back(IOThreads_.back()->getLoop());
    }
    started_ = true;
    LOG_INFO << "IO thread pool started.";
    //std::cout << "IO thread pool started." << std::endl;
}
void IOThreadPool::stop()
{
    for (auto &thread : IOThreads_)
    {
        thread->join();
    }
    //TODO:log_info<<"IO thread pool stopped."
    LOG_INFO << "IO thread pool stopped.";
}
void IOThreadPool::put(Task task)
{
    if(tid_==bases::currentThreadID()&&started_)
    {
        loops_[next_++]->runInLoop(task);
    }
    else
    {
        //TODO:log_fatal<<"Not in owner thread.";
        LOG_FATAL << "Not in owner thread.";
        //throw;
    }
}
} // namespace event
