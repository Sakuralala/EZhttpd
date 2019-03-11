/**
 *测试muduo中所说的copy-on-write的比例,具体来说测试任务如下:把"man open"输出到一个文件(就是
 *想随便找一个单词量适中的以便于测试)中，并将其内部的每个单词存入vector<string>(看了下总共记
 *了刚好1000个单词),线程池开始工作后，主线程不断地往ThreadPoolTest实例中写"string-int"对(随
 *机),并不断往task queue中添加任务(随机get一个"string-int"对,和前面那个随机的对不一定一样),
 *然后主要是为了测试muduo上所说的copy-on-write所占的比例(读的时候使用了一个临时对象来增加实
 *际数据的引用计数，然后马上释放锁，尽量减小临界区的大小，以便其他读写线程可以工作，这个测的
 *就是有子线程在读的情况下主线程恰好要写的比例)。  
*/
#include "bases/threadPool.h"
#include "bases/threadPoolTest.h"
#include <ctime> //for threadpool test.
#include <iostream>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
using namespace bases;
using namespace event;
using std::cout;
using std::endl;
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
EventLoop *globalLoop = nullptr;
int main()
{
    int cnt = 500000;
    ThreadPool tp(1024);
    ProducerConsumer pc;
    ifstream inFile("miscs/open");
    vector<string> vs = getString(inFile);
    //string str("strrrr.");
    //int num = -1;
    //主线程不断往taskQueue中put任务(pc的get) 且不断put随机一个string->number对，其余子线程不断get随机一个string 并返回其number
    //Thread t1(std::bind(&ProducerConsumer::put,&pc,std::ref(str)));
    tp.run(4);
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
}