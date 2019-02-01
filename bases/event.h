#ifndef _event_h
#define _event_h
/**
 *事件的抽象，包括对应的文件描述符(用于epoll的监控)、相应的事件回调、用户读写缓冲区； 
*/
#include <functional>
#include "mutex.h"
namespace event
{
class EventLoop;
class Event : Nocopyable
{
  public:
    typedef std::function<void()> Callback;
    Event(int fd, EventLoop *loop);
    Event(Event &&ev);
    ~Event();
    //处理事件的核心
    void handleEvent();
    /**
     *note:以下几个方法均不是线程安全的；如果正在执行某个callback,而某个线程又调用了set*函数会引
    *发UB; 但是由于Event类一般是作为上层类的成员，所以一般没啥问题；而且一般只在开始时set一下
    */
    void setReadCallback(Callback cb)
    {
        readCallback_ = std::move(cb);
    }
    void setWriteCallback(Callback cb)
    {
        writeCallback_ = std::move(cb);
    }
    void setErrorCallback(Callback cb)
    {
        errorCallback_ = std::move(cb);
    }
    void setReadyType(int rt)
    {
        readyType_ = rt;
    }
    void setOperation(int op)
    {
        operation_ = op;
    }
    //严格来说get*也存在race condition,有可能其他线程调用get*而owner线程正在set,不过一般而言其他线程不会进行get*操作(没啥意义)
    EventLoop *getLoop() const
    {
        return loop_;
    }
    int getFd() const
    {
        return fd_;
    }
    uint32_t getInterestedType() const
    {
        return interestedType_;
    }
    int getOperation() const
    {
        return operation_;
    }
    /**
     *note:以下enable系列函数均非线程安全，其他线程的唯一调用方式就是使用runInLoop,否则出
     *错; 虽说epoll相关函数是线程安全的(红黑树通过mutex保护，readylist通过自旋锁保护)，但
     *是这里需要保证interestedType_以及operation_的线程安全性，这里不用锁而是改用assert强
     *制要求在owner线程，否则直接报错；
     *另外，enable/disable*都需要std::bind()来调用的话感觉很奇怪
    */
    void enableRead()
    {
        interestedType_ |= ReadEvent;
        update();
    }
    void enableWrite()
    {
        interestedType_ |= WriteEvent;
        update();
    }
    void disableRead()
    {
        interestedType_ &= ~ReadEvent;
        update();
    }
    void disableWrite()
    {
        interestedType_ &= ~WriteEvent;
        update();
    }
    void enableAll()
    {
        interestedType_ |= (ReadEvent | WriteEvent);
        update();
    }
    void disableAll()
    {
        interestedType_ = 0;
        update();
    }
    //从epoll监控中删除
    void remove();

  private:
    static const int ReadEvent;
    static const int WriteEvent;
    //NOTE:当前event并不拥有fd
    int fd_;
    /******另一种做法:使用mutex保护下面三个成员****/
    //bases::Mutex mutex_;
    //要在epoll对象中执行的操作(增删改)
    int operation_;
    //当前已经就绪的事件类型
    int readyType_;
    //感兴趣的事件类型
    uint32_t interestedType_;
    //这个事件所属的循环
    //由于EventLoop和Event类相互引用，所以只能用指针/引用
    //loop的生命期保证比event长，且loop的生命期不由event管理，所以此处使用non-owing raw pointers
    EventLoop *loop_;
    Callback readCallback_;
    Callback writeCallback_;
    Callback errorCallback_;
    //TODO:buffer
    /************************以下为内部调用*******************************************/
    //在所属的eventloop对应的io线程中的epoll对象中增加、修改、删除对自身的监控
    void update();
};
} // namespace event
#endif // !_connection_h
