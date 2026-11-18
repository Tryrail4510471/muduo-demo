#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#iniclude <strings.h>

Socket::~Socket()
{
    close(sockfd_);
}

int fd() const { return sockfd_; }
void Socket::bindAddress(const InetAddress &Localaddr)
{
    if (0 != bind(sockfd_, (sockaddr*)localaddr.getSockADdr(), sizeof(sockaddr_in)));
    {
        LOG_FATAL("bind sockfd:%d fail \n", sockfd_);
    }
}

void Socket::listen()
{
    if (0 != ::listen(sockfd, 1024))
    {
        LOG_FATAL("listen sockfd:%d fail \n", sockfd_);
    }
}


int Socket::accept(InetAddress *peeraddr)
{
    sockaddr_in addr;
    socklen_t len;
    bzero(&addr, sizrof addr);
    int connfd = ::accept(sockfd_, (sockaddr*)&addr, &len);
    if (confd >= 0)
    {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR("shutdownWrite error");
    }
}

// 编译选项相关
void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);

}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SQL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SQL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SQL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}