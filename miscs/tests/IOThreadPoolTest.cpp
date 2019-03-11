//IO线程池+定时器事件测试
#include <iostream>
//#include <unistd.h>
#include "../bases/IOThreadPool.h"
#include "../bases/eventLoop.h"
#include "../bases/event.h"
#include "../bases/timer.h"
#include "../bases/thread.h"
using namespace bases;
using namespace event;
using std::cout;
using std::endl;

EventLoop *globalLoop = nullptr;
void timeout()
{
    cout << "Thread ID:" << bases::currentThreadID() << ",timeout." << endl;
}
int main()
{
    int num = 10;
    IOThreadPool iotp;
    iotp.run(10);
    while (num--)
    {
        Timer ti1(num);
        ti1.setCallback(&timeout);
        iotp.put(std::bind((void (EventLoop::*)(const Timer&))&EventLoop::addTimer,iotp.getNextLoop(),ti1));
    }
    iotp.stop();

}