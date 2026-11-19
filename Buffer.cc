#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

/**
 * 从fd上读取数据 Poller工作在LT模式 不断响应直到读取完毕
 * Buffer缓冲区是有大小的! 但是从fd上读数据的时候,却不知道Tcp数据的最终大小
 */

ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrabuf[65536] = {0}; //  栈上的内存空间, 64*1024 bit
    struct iovec vec[2];
    const size_t writable = writableBytes(); // 这是Buffer底层缓冲区剩余的可写空间大小
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = :: readv(fd, vec, iovcnt);   // 系统调用放入数据   
    // readv可以从文件描述符中读出数据到多个分散缓冲区下
    // 分散读取:分散填充到多个不连续的缓冲区中。它相当于一次系统调用完成多次 read 操作
    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if (n <= writable) // buffer的可写缓冲区已经足够 存储读出的数据了
    {
        writerIndex_ += n;
    }
    else // extrabuf里面也写入了数据
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable); // writerIndex_开始写 n -writable大小的数据 
    }

    return n; // 读取的字节数
}

ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}