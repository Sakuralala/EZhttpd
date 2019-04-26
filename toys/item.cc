#include <string.h>
#include <functional>
#include "item.h"

namespace toys
{
Item::Item(const string &key, uint32_t valLen, uint32_t flags, uint32_t cas) : keyLen_(key.size()), valLen_(valLen), curDataLen_(0), casVal_(cas), flags_(flags), hashVal_(std::hash<string>{}(key)), data_(new char(keyLen_ + valLen_))
{
    appendData(key);
}
bool Item::appendData(const string &newData)
{
    if (remainLength() < newData.size())
        return false;
    memcpy(data_ + curDataLen_, newData.c_str(), newData.size());
    curDataLen_ += newData.size();
    return true;
}
} // namespace toys
