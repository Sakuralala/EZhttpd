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
    ifstream inFile("miscs/open");
    vector<string> vs = getString(inFile);
    AsyncLog logging("./logfile/");
    logging.run();
    for (auto &str : vs)
    {
        logging.append(str.c_str(), str.size());
    }
    logging.stop();
}