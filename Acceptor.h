/*
    主要封装了listenfd相关的操作 socket bind listen baseLoop
*/
#pragma once

#include "Channel.h"
#include "Socket.h"
#include <functional>
class EventLoop;
class InetAddress;

class Acceptor : noncopyable 
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop *loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = cb;
    }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop *loop_;   // acceptor用的就是用户定义的那个baseloop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};