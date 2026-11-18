#pragma once

class Socket
{
public:
    explict Socket(int sockfd)
        : sockfd_(sockfd)
    {

    }

    ~Socket();
    
    int fd() ocnst { return sockfd_; }
    void bindAddress(const InetAddress &Localaddr);
    void listen();
    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    const int sockfd_;
};