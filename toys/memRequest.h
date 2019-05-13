#ifndef memrequest_h
#define memrequest_h

#include <memory>
#include <string>
#include "item.h"
#include "memcachedServer.h"
#include "../net/connection.h"
#include "../bases/circularBuffer.h"

namespace toys
{
class MemRequest
{
public:
    enum State
    {
        REQUEST_LINE,
        VALUE,
        QUIT
    };
    MemRequest(const net::ConnectionPtr &conn, MemcachedServer *server);
    ~MemRequest();
    State parseandReply(bases::UserBuffer &buf);
    void parseCommandLine(const string &str);

private:
    void resetRequest()
    {
        state_ = REQUEST_LINE;
        policy_ = Item::Policy::UNKNOWN;
        noReply_ = false;
        item_.reset();
    }
    std::weak_ptr<net::Connection> owner_;
    MemcachedServer *server_;
    State state_;
    bool noReply_;
    Item::Policy policy_;
    ItemPtr item_;
};
} // namespace toys
#endif // !memrequest_h