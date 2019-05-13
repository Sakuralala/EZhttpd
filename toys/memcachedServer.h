#ifndef memcachedserver_h
#define memcachedserver_h
/**
 *memcached server. 
*/
#include "item.h"
#include "../net/server.h"

namespace toys
{
class MemcachedServer : public net::Server
{
public:
    MemcachedServer(event::EventLoop *loop, const std::vector<int> &ports);
    ~MemcachedServer() = default;
    void run(int numThreads)
    {
        net::Server::run(numThreads);
    }
    void stop()
    {
        net::Server::stop();
    }
    void onMessage(const net::Server::ConnectionPtr &conn, bases::UserBuffer &buf);
    void onClose(const net::Server::ConnectionPtr &conn);
    //这组get/set是为了测试减小引用计数+创建新item的耗时
    bool setItem(const ItemPtr &item, Item::Policy pol);
    ItemPtr getItem(const ItemPtr &key);
    //以下这组get/set是为了测试拷贝原始value的耗时
    bool setItemwithoutNew(const ItemPtr &item, Item::Policy pol);
    Item getItemwithCopy(const ItemPtr &key);
    bool delItem(const ItemPtr &key);

private:
    //net::Server server_;
    static const int ShardNumber = 2048;
    ItemswithLock itemsArr_[ShardNumber];
};
} // namespace toys
#endif // !1