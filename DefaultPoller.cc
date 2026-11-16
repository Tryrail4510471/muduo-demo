#include "Poller.h"

#include <stdlib.h>

Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLLER"))
    {
        return nullptr;    //生成 poll的实例
    }
    else
    {   
        return nullptr;     // 生成epoll实例
    }
}