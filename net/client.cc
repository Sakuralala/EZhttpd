#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.h"
#include "connection.h"
#include "../bases/utils.h"
#include "../log/logger.h"

namespace net
{
Client::Client(event::EventLoop *loop) : connected_(false), retryTimes_(0), loop_(loop), fd_(-1)
{
    if (!loop)
        LOG_FATAL << "Event loop can't be null.";
    setCloseCallback(std::bind(&net::Client::defaultCloseCallback, this, std::placeholders::_1));
}
void Client::defaultCloseCallback(const ConnectionPtr &conn)
{
    //assert: connected == true.
    if (connected_)
    {
        LOG_DEBUG << "Close fd: " << fd_;
    }
}

bool Client::connect(const std::string &peer)
{
    loop_->assertInOwnerThread();
    if (connected_)
        disconnect();
    fd_ = bases::createNonBlockingSocket();
    if (fd_ == -1)
        return false;
    auto ipport(bases::split(peer, ':'));
    if (ipport.size() != 2)
        return false;
    //0 is returned if src does not contain a character string rep‐
    //resenting a valid network address in the specified address family.
    if (!inet_pton(AF_INET, ipport[0].c_str(), &peerAddr_.sin_addr))
    {
        LOG_ERROR << "Invalid IP address:" << ipport[0];
        return false;
    }
    peerAddr_.sin_family = AF_INET;
    //std::invalid_argument is throwed if wrong.
    peerAddr_.sin_port = htons(std::stoi(ipport[1]));
    return _connect();
}
bool Client::connect(const std::string &peerIP, int port)
{
    loop_->assertInOwnerThread();
    if (connected_)
        disconnect();
    fd_ = bases::createNonBlockingSocket();
    if (fd_ == -1)
        return false;
    if (!inet_pton(AF_INET, peerIP.c_str(), &peerAddr_.sin_addr))
    {
        LOG_ERROR << "Invalid IP address:" << peerIP;
        return false;
    }
    peerAddr_.sin_family = AF_INET;
    peerAddr_.sin_port = htons(port);
    return _connect();
}

bool Client::connect(sockaddr_in &peerAddr)
{
    loop_->assertInOwnerThread();
    if (connected_)
        disconnect();
    fd_ = bases::createNonBlockingSocket();
    if (fd_ == -1)
        return false;
    peerAddr_ = peerAddr;
    return _connect();
}
bool Client::_connect()
{
    auto ret = ::connect(fd_, (sockaddr *)&peerAddr_, sizeof(sockaddr_in));
    if (ret == -1)
    {
        switch (errno)
        {
        //still connecting.
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting();
            break;
        //connection failed immediately.
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry();
            break;
        //Other errors.
        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_ERROR << "Connection error: " << strerror(errno);
            ::close(fd_);
            return false;
            break;

        default:
            LOG_ERROR << "Unexpected error.";
            ::close(fd_);
            return false;
        }
    }
    else //connection established immediately.
    {
        connectionEstablished();
    }
    return true;
}
//still connecting, register a callback.
void Client::connecting()
{
    connectingEvent_.reset(new event::Event(fd_, loop_));
    connectingEvent_->setWriteCallback(std::bind(&net::Client::connectionEstablished, this));
    connectingEvent_->setReadCallback(std::bind(&net::Client::connectionFailed, this));
    //when connected, the write event is ready;if connection failed, the read event is ready;
    connectingEvent_->enableAll();
}
//connection failed, try again.
void Client::retry()
{
    ::close(fd_);
    if (++retryTimes_ <= MaxRetryTimes)
    {
        auto addr(getPeerAddress());
        LOG_INFO << "Retry connecting to " << addr.first << ":" << addr.second << ". Retry times(including this time): " << retryTimes_;
        connect(peerAddr_);
    }
    else
    {
        LOG_ERROR << "Retried " << retryTimes_ << ", give up.";
    }
}
//client close connection proactively. Both read and write parts are closed.
void Client::disconnect()
{
    loop_->assertInOwnerThread();
    if (connected_)
    {
        connected_ = false;
        //LOG_INFO << "Current reference count:" << conn_.use_count();
        conn_.reset();
        LOG_DEBUG << "Close connection by client.";
    }
}
std::pair<std::string, uint16_t> Client::getLocalAddress() const
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &localAddr_.sin_addr, buf, INET_ADDRSTRLEN);
    return std::pair<std::string, uint16_t>(buf, ntohs(peerAddr_.sin_port));
}
std::pair<std::string, uint16_t> Client::getPeerAddress() const
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peerAddr_.sin_addr, buf, INET_ADDRSTRLEN);
    //都是网络字节序的
    return std::pair<std::string, uint16_t>(buf, ntohs(peerAddr_.sin_port));
}
//Before connection establised callbacks: connection established; connection failed.
void Client::connectionEstablished()
{
    socklen_t localAddrLen = sizeof(localAddr_);
    if (getsockname(fd_, (struct sockaddr *)&localAddr_, &localAddrLen))
    {
        LOG_ERROR << "Get sock name error in socket:" << fd_;
        ::close(fd_);
        resetConnectingEvent();
        return;
    }
    LOG_DEBUG << "Connect success.";
    connected_ = true;
    connectingEvent_->remove();
    conn_.reset(new Connection(loop_, fd_, localAddr_, peerAddr_));
    conn_->setWriteCallback(writeCallback_);
    conn_->setReadCallback(readCallback_);
    conn_->setCloseCallback(closeCallback_);
    //owner thread.
    conn_->eventInit();
    //still under connectingEvent_ call schedule:
    //Event::handleEvent->Client::connectionEstablished; so reset should run later.
    loop_->queueInLoop(std::bind(&net::Client::resetConnectingEvent, this));
    if (connectionCallback_)
        connectionCallback_(conn_);
}

void Client::connectionFailed()
{
    auto addr(getPeerAddress());
    LOG_ERROR << "Connecting to " << addr.first << ":" << addr.second << " error:" << strerror(errno) << ", retrying.....";
    retry();
}

void Client::resetConnectingEvent()
{
    if (connectingEvent_.get())
    {
        connectingEvent_.release();
    }
}
} // namespace net