#ifndef __processState_h
#define __processState_h

#include <string>
#include <vector>
namespace toys
{
//NOTE:
//由于sysMonitor的标准格式为/module/command/args...,查看某个进程的话，这里暂时就通过
//args中携带pid或者进程名字来代替了。
struct ProcessStatus
{
    typedef std::vector<std::string> Args;
    std::string overview(const Args &args);
    std::string pid(const Args &args);
    std::string threadsStatus(const Args &args);
    std::string procStatus(const Args &args);
};
} // namespace toys

#endif