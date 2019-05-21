#include "../bases/fileUtil.h"
#include "processStatus.h"

namespace toys
{
std::string ProcessStatus::overview(const Args &args)
{
    std::string ret;
    std::string name("/proc" + args[0] + "/status");
    bases::ReadFile(name).readtoString(ret);
    return ret;
}
std::string ProcessStatus::pid(const Args &args)
{
}
std::string ProcessStatus::procStatus(const Args &args)
{
    std::string ret;
    std::string name("/proc" + args[0] + "/status");
    bases::ReadFile(name).readtoString(ret);
    return ret;
}
std::string ProcessStatus::threadsStatus(const Args &args)
{
    std::string tasks;
    std::string name("/proc" + args[0] + "/task/");

}
} // namespace toys