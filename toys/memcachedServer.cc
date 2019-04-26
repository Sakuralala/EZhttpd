#include "memcachedServer.h"

namespace toys
{
MemcachedServer::MemcachedServer(event::EventLoop *loop, const std::vector<int> &ports) : server_(loop, ports)
{
}
ItemPtr MemcachedServer::getItem(const ItemPtr &key)
{
    bases::MutexGuard mg(itemsArr_[key->hash() % ShardNumber].mutex_);
    auto items = itemsArr_[key->hash() % ShardNumber].items_;
    auto ite =items.find(key);
    return ite == items.end() ? ItemPtr() : *ite;
}
bool MemcachedServer::setItem(const ItemPtr &item, Item::Policy pol)
{
}
Item MemcachedServer::getItemwithCopy(const ItemPtr &key)
{
    bases::MutexGuard mg(itemsArr_[key->hash() % ShardNumber].mutex_);
    auto items = itemsArr_[key->hash() % ShardNumber].items_;
    auto ite =items.find(key);
    return ite == items.end() ? Item() : **ite;
}
bool MemcachedServer::setItemwithoutNew(const ItemPtr &item, Item::Policy pol)
{
}
bool MemcachedServer::delItem(const ItemPtr &key)
{
    bases::MutexGuard mg(itemsArr_[key->hash() % ShardNumber].mutex_);
    return itemsArr_[key->hash() % ShardNumber].items_.erase(key) == 1;
}
} // namespace toys
