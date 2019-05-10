#include <string.h>
#include <functional>
#include <utility>
#include <algorithm>
#include <atomic>
#include "item.h"

namespace toys
{
std::atomic_int64_t globalCasVal = ATOMIC_VAR_INIT(0);
Item::Item(const string &key, size_t valLen, size_t flags) : keyLen_(key.size()), valLen_(valLen), curDataLen_(0), flags_(flags), casVal_(std::atomic_fetch_add(&globalCasVal, static_cast<long>(1)) + 1), hashVal_(std::hash<string>{}(key)), data_(new char[keyLen_ + valLen_])
{
    appendData(key, key.size());
}
bool Item::appendData(string &&newData, size_t len)
{
    if (remainLength() < len)
        return false;
    memcpy(data_ + curDataLen_, newData.c_str(), len);
    curDataLen_ += len;
    return true;
}
bool Item::appendData(const string &newData, size_t len)
{
    if (remainLength() < len)
        return false;
    memcpy(data_ + curDataLen_, newData.c_str(), len);
    curDataLen_ += len;
    return true;
    //FIXME:左值引用的参数的函数中如何转调用右值引用的同名函数？
    //return appendData(std::move(newData), len);
}
bool Item::prependData(string &&newData, size_t len)
{
    if (remainLength() < len)
        return false;
    std::copy_backward(data_ + keyLen_, data_ + curDataLen_, data_ + len);
    memcpy(data_ + keyLen_, newData.c_str(), len);
    return true;
}
bool Item::prependData(const string &newData, size_t len)
{
    if (remainLength() < len)
        return false;
    std::copy_backward(data_ + keyLen_, data_ + curDataLen_, data_ + len);
    memcpy(data_ + keyLen_, newData.c_str(), len);
    return true;
}
} // namespace toys
