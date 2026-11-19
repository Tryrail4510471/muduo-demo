#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    // 避免重复定义导致链接失败:外部变量在.h中声明，在.c中定义
    extern __thread int t_cachedTid;

    void cachedTid();

    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cachedTid();
        }
        return t_cachedTid;
    }
}