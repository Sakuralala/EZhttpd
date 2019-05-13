#include <iostream>
#include "memcachedServer.h"
<<<<<<< HEAD
#include "memRequest.h"
#include "../log/logger.h"
#include "../net/connection.h"
=======
#include <iostream>

>>>>>>> effb2ec57d50be33aac302c9639036cc5c445937
namespace toys
{
MemcachedServer::MemcachedServer(event::EventLoop *loop, const std::vector<int> &ports) : net::Server(loop, ports)
{
    setReadCallback(std::bind(&toys::MemcachedServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    setCloseCallback(std::bind(&toys::MemcachedServer::onClose, this, std::placeholders::_1));
}
void MemcachedServer::onClose(const net::Server::ConnectionPtr &conn)
{
    delConnection(conn->getFd());
}
void MemcachedServer::onMessage(const net::Server::ConnectionPtr &conn, bases::UserBuffer &buf)
{
    LOG_DEBUG << "New request accepted.";
    auto currentLoop = conn->getLoop();
    if (!currentLoop)
    {
        LOG_ERROR << "No event loop owns this connection,closed.";
        //服务端主动关闭连接
        conn->handleClose();
        return;
    }
    MemRequest *req = std::any_cast<MemRequest>(conn->getContext());
    //首先创建一个http request请求的上下文
    if (!req)
    {
        conn->setContext(MemRequest(conn, this));
        req = std::any_cast<MemRequest>(conn->getContext());
    }
    auto state = req->parseandReply(buf);
    if (state == MemRequest::State::QUIT)
        conn->handleClose();
}
ItemPtr MemcachedServer::getItem(const ItemPtr &key)
{
    auto &items = itemsArr_[key->hash() % ShardNumber].items_;
    bases::MutexGuard mg(itemsArr_[key->hash() % ShardNumber].mutex_);
    auto ite = items.find(key);
    return ite == items.end() ? ItemPtr() : *ite;
}
//用于get/set比值高的情景，通过把get的临界区的耗时转移到set中来提高性能(set通过先创建新的再删除旧的的方式来解决get/set的race condition)
bool MemcachedServer::setItem(const ItemPtr &item, Item::Policy pol)
{
    auto &items = itemsArr_[item->hash() % ShardNumber].items_;
    bases::MutexGuard mg(itemsArr_[item->hash() % ShardNumber].mutex_);
    auto ite = items.find(item);
    if (pol == Item::Policy::SET || pol == Item::Policy::ADD || pol == Item::Policy::REPLACE)
    {
        if (ite != items.end())
        {
            if (pol == Item::Policy::ADD)
                return false;
            items.erase(ite);
        }
        items.insert(item);
        //std::cout << item.use_count() << std::endl;
        return true;
    }
    else if (pol == Item::Policy::APPEND || pol == Item::Policy::PREPEND)
    {
        if (item->curValLen() == 0)
            return true;
        size_t len = (*ite)->endwithCRLF() ? item->curValLen() - 2 : item->curValLen();
        if (ite == items.end() || (*ite)->remainLength() < len)
            return false;
        //TODO:casVal
        auto newItemPtr(std::make_shared<Item>(item->key(), (*ite)->valLen(), item->flag()));
        if (pol == Item::Policy::APPEND)
        {
            if ((*ite)->endwithCRLF())
                newItemPtr->appendData((*ite)->data(), (*ite)->curValLen() - 2);
            newItemPtr->appendData(item->data(), item->curValLen());
        }
        else
        {
            if (item->endwithCRLF())
                newItemPtr->appendData(item->data(), item->curValLen() - 2);
            newItemPtr->appendData((*ite)->data(), (*ite)->curValLen());
        }
        items.erase(ite);
        items.insert(newItemPtr);
        return true;
    }
    return false;
}
Item MemcachedServer::getItemwithCopy(const ItemPtr &key)
{
    bases::MutexGuard mg(itemsArr_[key->hash() % ShardNumber].mutex_);
    auto &items = itemsArr_[key->hash() % ShardNumber].items_;
    auto ite = items.find(key);
    return ite == items.end() ? Item() : **ite;
}
//把临界区的耗时留在get那部分
bool MemcachedServer::setItemwithoutNew(const ItemPtr &item, Item::Policy pol)
{
    auto &items = itemsArr_[item->hash() % ShardNumber].items_;
    bases::MutexGuard mg(itemsArr_[item->hash() % ShardNumber].mutex_);
    auto ite = items.find(item);
    if (pol == Item::Policy::SET || pol == Item::Policy::ADD || pol == Item::Policy::REPLACE)
    {
        if (ite != items.end())
        {
            //TODO:长度超过现有的容量时该直接返回还是先delete再insert？
            if (pol == Item::Policy::ADD || item->curValLen() > (*ite)->valLen())
                return false;
        }
        (*ite)->resetVal();
        (*ite)->appendData(item->data(), item->curValLen());
        return true;
    }
    else if (pol == Item::Policy::APPEND || pol == Item::Policy::PREPEND)
    {
        size_t len = (*ite)->endwithCRLF() ? (*ite)->remainLength() + 2 : (*ite)->remainLength();
        if (ite != items.end() && item->curValLen() <= len)
        {
            if (pol == Item::Policy::APPEND)
                return (*ite)->appendData(item->data(), item->curValLen());
            else
                return (*ite)->prependData(item->data(), item->curValLen());
        }
    }
    return false;
}
bool MemcachedServer::delItem(const ItemPtr &key)
{
    bases::MutexGuard mg(itemsArr_[key->hash() % ShardNumber].mutex_);
    return itemsArr_[key->hash() % ShardNumber].items_.erase(key) == 1;
}
} // namespace toys
