/**
 *memcached server. 
*/

#include "item.h"
#include "../net/server.h"

namespace toys
{
class MemcachedServer
{
public:
    MemcachedServer(event::EventLoop *loop, const std::vector<int> &ports);
    ~MemcachedServer() = default;
    void run(int numThreads)
    {
        server_.run(numThreads);
    }
    void stop()
    {
        server_.stop();
    }
    //这组get/set是为了测试减小引用计数+创建新item的耗时
    bool setItem(const ItemPtr &item, Item::Policy pol);
    ItemPtr getItem(const ItemPtr &key);
    //以下这组get/set是为了测试拷贝原始value的耗时
    bool setItemwithoutNew(const ItemPtr &item, Item::Policy pol);
    Item getItemwithCopy(const ItemPtr &key);
    bool delItem(const ItemPtr &key);

private:
    net::Server server_;
    static const int ShardNumber = 2048;
    ItemswithLock itemsArr_[ShardNumber];
};
} // namespace toys