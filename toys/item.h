/**
 * memcached toy version. 
 * Item is the basic datastructure.
*/
#include <stdlib.h>
#include <memory>
#include <string>
#include <unordered_set>
#include "../bases/mutex.h"
namespace toys
{
using std::shared_ptr;
using std::string;
class Item
{
public:
    enum Policy
    {
        SET,
        ADD,
        REPLACE,
        APPEND,
        PREPEND,
        CAS
    };
    Item (){}
    Item(const string &key, uint32_t valLen,uint32_t flags, uint32_t casVal=1);
    ~Item()
    {
        ::delete data_;
    }
    uint32_t currentLength() const
    {
        return curDataLen_;
    }
    uint32_t totalLength() const
    {
        return keyLen_ + valLen_;
    }
    uint32_t remainLength() const
    {
        return totalLength() - curDataLen_;
    }
    string key() const
    {
        return string(data_, keyLen_);
    }
    string data() const
    {
        return string(data_ + keyLen_, curDataLen_-keyLen_);
    }
    uint32_t flag() const
    {
        return flags_;
    }
    uint32_t cas() const
    {
        return casVal_;
    }
    uint32_t hash() const
    {
        return hashVal_;
    }
    bool appendData(const string &newData);

private:
    uint32_t keyLen_;
    uint32_t valLen_;
    uint32_t curDataLen_;
    uint32_t casVal_;
    uint32_t flags_;
    size_t hashVal_;
    char *data_;
};
typedef shared_ptr<Item> ItemPtr;
typedef shared_ptr<const Item> ConstItemPtr;
struct ItemHash
{
    size_t operator()(const ItemPtr &ptr)
    {
        return ptr->hash();
    }
};
struct ItemCmp
{
    bool operator()(const ItemPtr &ptr1,const ItemPtr &ptr2)
    {
        return ptr1->key() == ptr2->key();
    }
};
typedef std::unordered_set<ItemPtr, ItemHash, ItemCmp> ItemSet;
//带锁的一个itemset
struct  ItemswithLock
{
    ItemSet items_;
    bases::Mutex mutex_;
};
} // namespace toys