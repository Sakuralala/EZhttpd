//#include "threadPool.h"
//#include "threadPoolTest.h"
//#include <ctime> //for threadpool test.
#include <iostream>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>
//#include <fstream>
//#include <sstream>
//#include <stdlib.h>
#include "IOThread.h"
#include "eventLoop.h"
#include "event.h"
using namespace bases;
using namespace event;
using std::cout;
using std::endl;
/*
using std::ifstream;
using std::istringstream;

vector<string> getString(ifstream &inFile)
{
    string line, word;
    vector<string> ret;
    while(getline(inFile,line))
    {
        istringstream record(line);
        while(record>>word)
            ret.push_back(word);
    }
    return ret;
}
*/
EventLoop *globalLoop = nullptr;
int *globalFd = nullptr;
void loopFunc()
{
    globalLoop->loop();
}
void timeout()
{
    uint64_t one = 0;
    if(::read(*globalFd,&one,sizeof one)!=sizeof one)
        throw;
        
    cout << "Thread ID:" << bases::currentThreadID() << ",timeout." << endl;
    //globalLoop->quit();
}
int main()
{
    cout << "thread id:" << bases::currentThreadID() << endl;
    //TODO:多次定时
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    globalFd = &timerfd;
    struct itimerspec tm;
    memset(&tm,0,sizeof tm);
    //首次触发
    tm.it_value.tv_sec = 2;
    //周期性触发
    tm.it_interval.tv_sec = 1;
    IOThread t1;
    t1.run();
    globalLoop = t1.getLoop();
    event::Event ev(timerfd, globalLoop);
    ev.setReadCallback(&timeout);
    //保证对epoll对象的更新由owner线程完成
    globalLoop->runInLoop(std::bind(&event::Event::enableRead,&ev));
    ::timerfd_settime(timerfd,0,&tm,nullptr);
    t1.join();
    ::close(timerfd);
    /*
    int cnt = 500000;
    ThreadPool tp(1024);
    ProducerConsumer pc;
    ifstream inFile("miscs/open");
    vector<string> vs = getString(inFile);
    //string str("strrrr.");
    //int num = -1;
    //主线程不断往taskQueue中put任务(pc的get) 且不断put随机一个string->number对，其余子线程不断get随机一个string 并返回其number
    //Thread t1(std::bind(&ProducerConsumer::put,&pc,std::ref(str)));
    tp.start(4);
    while (cnt)
    {
        srandom(random());
        int index = random() % vs.size(), num = random() % cnt;

        pc.put(vs[index], num);
        tp.put(std::bind(&ProducerConsumer::get, &pc, std::ref(vs[random() % vs.size()])));
        cnt--;
    }
    tp.stop(ExitMode::Graceful);
    std::cout << "Copy on write:" << pc.copyonWriteRate() << std::endl;
    */
    /*测试event loop在非其所属线程中进行调用时是否能够按预期所终止
    EventLoop loop;
    globalLoop = &loop;
    Thread t(loopFunc,"worker");
    t.run();
    getchar();
    */
}