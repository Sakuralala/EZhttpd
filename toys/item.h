/**
 * memcached toy version. 
 * Item is the basic datastructure.
*/
#ifndef item_h
#define item_h

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
        CAS,
        UNKNOWN
    };
    Item() {}
    Item(const string &key, size_t valLen, size_t flags);
    ~Item()
    {
<<<<<<< HEAD
        ::delete[] data_;
=======
        ::delete [] data_;
>>>>>>> effb2ec57d50be33aac302c9639036cc5c445937
    }
    size_t curDataLen() const
    {
        return curDataLen_;
    }
    size_t curValLen() const
    {
        return curDataLen_ - keyLen_;
    }
    size_t totalLen() const
    {
        return keyLen_ + valLen_;
    }
    size_t remainLength() const
<<<<<<< HEAD
    {
        return totalLen() - curDataLen_;
    }
    size_t keyLen() const
    {
        return keyLen_;
    }
    size_t valLen() const
    {
=======
    {
        return totalLen() - curDataLen_;
    }
    size_t keyLen() const
    {
        return keyLen_;
    }
    size_t valLen() const
    {
>>>>>>> effb2ec57d50be33aac302c9639036cc5c445937
        return valLen_;
    }
    string key() const
    {
        return string(data_, keyLen_);
    }
    string data() const
    {
        return string(data_ + keyLen_, curDataLen_ - keyLen_);
    }
    size_t flag() const
    {
        return flags_;
    }
    size_t cas() const
    {
        return casVal_;
    }
    size_t hash() const
    {
        return hashVal_;
    }
    void resetVal()
    {
        curDataLen_ = keyLen_;
    }
    bool endwithCRLF() const
    {
        return curDataLen_ >= 2 && (*(data_ + curDataLen_ - 1) == '\n') && (*(data_ + curDataLen_ - 2) == '\r');
    }
    bool appendData(const string &newData, size_t len);
    bool appendData(string &&newData, size_t len);
    bool prependData(const string &newData, size_t len);
    bool prependData(string &&newData, size_t len);

private:
    size_t keyLen_;
    size_t valLen_;
    size_t curDataLen_;
    size_t flags_;
    size_t casVal_;
    size_t hashVal_;
    char *data_;
};
typedef shared_ptr<Item> ItemPtr;
typedef shared_ptr<const Item> ConstItemPtr;
struct ItemHash
{
    size_t operator()(const ItemPtr &ptr) const
    {
        return ptr->hash();
    }
};
struct ItemCmp
{
    bool operator()(const ItemPtr &ptr1, const ItemPtr &ptr2) const
    {
        return ptr1->key() == ptr2->key();
    }
};
typedef std::unordered_set<ItemPtr, ItemHash, ItemCmp> ItemSet;
//带锁的一个itemset
struct ItemswithLock
{
    ItemSet items_;
    bases::Mutex mutex_;
};
} // namespace toys
#endif // !item