#include "Poller.h"
#include "EPollPoller.h"

#include <stdlib.h>

Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLLER"))
    {
        return nullptr;    //生成 poll的实例
    }
    else
    {   
        return new EPollPoller(loop);     // 生成epoll实例
    }
}