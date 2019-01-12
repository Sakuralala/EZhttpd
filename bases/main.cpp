#include <string>
#include <vector>
#include <iostream>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "asyncLog.h"
#include "thread.h"
using namespace bases;
using std::cout;
using std::endl;
using std::ifstream;
using std::istringstream;
using std::string;
using std::vector;
vector<string> getString(ifstream &inFile)
{
    string line, word;
    vector<string> ret;
    while (getline(inFile, line))
    {
        istringstream record(line);
        while (record >> word)
            ret.push_back(word);
    }
    return ret;
}
AsyncLog *globalAsync = nullptr;
void logFunc(const char *msg, size_t len)
{
    globalAsync->append(msg, len);
}
int main()
{
    //同下  使用绝对路径
    ifstream inFile("/home/oldhen/cpptest/miscs/open");
    //vector<string> vs = getString(inFile);
    //fopen的相对路径中的当前目录在不同的环境下是不同的  
    //在命令行中直接调用可执行文件(或gdb)  相对路径的当前目录就是可执行文件所在的目录
    //但是在vscode里执行  相对路径的当前目录是工程的目录
    //所以切记使用绝对路径
    AsyncLog logging("/home/oldhen/cpptest/logfile/");
    logging.run();
    string line;
    while (getline(inFile, line))
    {
        if (line.empty())
            continue;
        line += '\n';
        logging.append(line.c_str(), line.size());
    }
    logging.stop();
}