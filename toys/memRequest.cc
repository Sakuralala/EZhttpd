#include <sstream>
#include <vector>
//#include <assert.h>
#include "memRequest.h"
#include "../log/logger.h"

namespace toys
{
MemRequest::MemRequest(const net::ConnectionPtr &conn, MemcachedServer *server) : owner_(conn), server_(server), state_(REQUEST_LINE), noReply_(false), policy_(Item::Policy::UNKNOWN)
{
    if (!server_)
        LOG_FATAL << "Memcached server can not be null!";
}
MemRequest::~MemRequest() = default;
MemRequest::State MemRequest::parseandReply(bases::UserBuffer &buf)
{
    while (true)
    {
        auto crlf = buf.findCRLF();
        if (crlf)
        {
            auto msg(buf.getMsg(buf.begin(), crlf));
            LOG_DEBUG << "Message:" << msg;
            if (state_ == REQUEST_LINE)
                parseCommandLine(buf.getMsg(buf.begin(), crlf));
            else if (state_ == VALUE)
            {
                if (item_)
                    item_->appendData(buf.getMsg(buf.begin(), crlf) + "\r\n", buf.length(buf.begin(), crlf) + 2);
                bool success = server_->setItem(item_, policy_);
                if (!noReply_)
                {
                    auto conn = owner_.lock();
                    if (conn)
                    {
                        if (success)
                            conn->send("STORED\r\n");
                        else
                        {
                            if (policy_ == Item::Policy::SET)
                                conn->send("ERROR\r\n");
                            else if (policy_ == Item::Policy::CAS)
                                conn->send("EXISTS or NOT_FOUND\r\n");
                            else
                                conn->send("NOT_STORED\r\n");
                        }
                    }
                }
                resetRequest();
            }
            buf.retrieve(buf.length(buf.begin(), crlf) + 2);
        }
        else
            break;
    }
    return state_;
}

void MemRequest::parseCommandLine(const std::string &str)
{
    if (str.empty())
    {
        auto conn = owner_.lock();
        if (conn)
            conn->send("Empty message.\r\n");
        resetRequest();
        return;
    }
    std::istringstream record(str);
    std::vector<std::string> words;
    std::string word;
    while (record >> word)
        words.push_back(word);
    if (words[0] == "set" || words[0] == "add" || words[0] == "replace" || words[0] == "append" || words[0] == "prepend")
    {
        if (words[0] == "set")
            policy_ = Item::Policy::SET;
        else if (words[0] == "add")
            policy_ = Item::Policy::ADD;
        else if (words[0] == "replace")
            policy_ = Item::Policy::REPLACE;
        else if (words[0] == "append")
            policy_ = Item::Policy::APPEND;
        else if (words[0] == "prepend")
            policy_ = Item::Policy::PREPEND;
        if (words.size() != 4 && words.size() != 5)
        {
            auto conn = owner_.lock();
            if (conn)
                conn->send("Parse command wrong.\r\n");
            resetRequest();
        }
        else
        {
            item_ = std::shared_ptr<Item>(new Item(words[1], std::stoul(words[3]) + 2, std::stoul(words[2])));
            if (words.size() == 5)
            {
                if (words.back() == "noreply")
                    noReply_ = true;
                else
                {
                    auto conn = owner_.lock();
                    if (conn)
                        conn->send("Parse command wrong.\r\n");
                    resetRequest();
                }
            }
            state_ = VALUE;
        }
    }
    else if (words[0] == "cas")
    {
        policy_ = Item::Policy::CAS;
        if (words.size() != 5 && words.size() != 6)
        {
            auto conn = owner_.lock();
            if (conn)
                conn->send("Parse command wrong.\r\n");
            resetRequest();
        }
        else
        {
            item_ = std::shared_ptr<Item>(new Item(words[1], std::stoul(words[3]) + 2, std::stoul(words[2])));
            item_->setCas(std::stoul(words[4]));
            if (words.size() == 6)
            {
                if (words.back() == "noreply")
                    noReply_ = true;
                else
                {
                    auto conn = owner_.lock();
                    if (conn)
                        conn->send("Parse command wrong.\r\n");
                    resetRequest();
                }
            }
            state_ = VALUE;
        }
    }
    else if (words[0] == "get" || words[0] == "gets")
    {
        if (words.size() == 1)
        {
            auto conn = owner_.lock();
            if (conn)
                conn->send("Lack of key.\r\n");
        }
        else
        {
            for (int i = 1; i < words.size(); ++i)
            {
                item_ = std::shared_ptr<Item>(new Item(words[i], 0, 0));
                auto item = server_->getItem(item_);
                auto conn = owner_.lock();
                if (conn)
                {
                    if (item)
                    {
                        if (words[0] == "get")
                        {
                            std::string reply("VALUE " + item->key() + " " + std::to_string(item->flag()) + " " + std::to_string(item->curValLen() - 2) + "\r\n" + item->data() + "END\r\n");
                            conn->send(reply);
                        }
                        else
                        {
                            std::string reply("VALUE " + item->key() + " " + std::to_string(item->flag()) + " " + std::to_string(item->curValLen() - 2) + " " + std::to_string(item->cas()) + "\r\n" + item->data() + "END\r\n");
                            conn->send(reply);
                        }
                    }
                    else
                        conn->send("VALUE\r\n\r\nEND\r\n");
                }
            }
        }
        resetRequest();
    }
    else if (words[0] == "delete")
    {
        if (words.size() != 2 && words.size() != 3)
        {
            auto conn = owner_.lock();
            if (conn)
                conn->send("Parse command wrong.\r\n");
        }
        else
        {
            noReply_ = words.size() == 3;
            item_ = std::shared_ptr<Item>(new Item(words[1], 0, 0));
            bool success = server_->delItem(item_);
            if (!noReply_)
            {
                auto conn = owner_.lock();
                if (conn)
                {
                    if (success)
                        conn->send("DELETED\r\n");
                    else
                        conn->send("NOT_FOUND\r\n");
                }
            }
        }
        resetRequest();
    }
    else if (words[0] == "quit")
    {
        if (words.size() != 1)
        {
            auto conn = owner_.lock();
            if (conn)
                conn->send("Parse command wrong.\r\n");
            resetRequest();
        }
        else
            state_ = QUIT;
    }
    else if (words[0] == "version")
    {
        auto conn = owner_.lock();
        if (conn)
        {
            if (words.size() != 1)
                conn->send("Parse command wrong.\r\n");
            else
                conn->send("Ver 0.01.\r\n");
        }
        resetRequest();
    }
    else if (words[0] == "all")
    {
        auto conn = owner_.lock();
        if (conn)
        {
            if (words.size() != 1)
                conn->send("Parse command wrong.\r\n");
            else
            {
                auto all(server_->getAll());
                if (all.empty())
                    conn->send("NONE\r\n");
                conn->send("TOTAL:" + std::to_string(all.size()) + "\r\n");
                for (auto item : all)
                {
                    std::string reply("VALUE " + item->key() + " " + std::to_string(item->flag()) + " " + std::to_string(item->curValLen() - 2) + "\r\n" + item->data() + "END\r\n");
                    conn->send(reply);
                }
            }
        }
        resetRequest();
    }
    else
    {
        auto conn = owner_.lock();
        if (conn)
            conn->send("Unknown command.\r\n");
        resetRequest();
    }
}
} // namespace toys