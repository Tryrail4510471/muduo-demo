/**
    ChannelList activeChannels_

    std::unique_ptr<Poller> poller
    int wakeupFd -> loop
    std::unique_ptr<Channel> wakeupChannel;
*/

#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Channel;
class Poller;

// 时间循环类 主要包含了两个大模块 Channel Poller (epoll的抽象)
class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // 开启事件循环
    void loop();
    // 退出事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_;}

    // 在当前loop中执行cb
    void runInLoop(Functor cb);
    // 吧cb放入队列中，唤醒loop「所在的线程，执行cb
    void queueInLoop(Functor cb);

    // 用来唤醒loop所在的线程
    void wakeup();

    // EventLoop的方法 ==》 Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    // 判断EventLoop的对象是否在自己的线程里面
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
private: 
    void handleRead();  // wakeup
    void doPendingFunctors();   // 执行回調

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;
    std::atomic_bool quit_;     // 标志退出loop循环
    std::atomic_bool callingPendingFunctors_;   //表示当前loop是否有需要执行的回調操作
    const pid_t threadId_;  //记录当前线程的ID
    Timestamp pollReturnTime_;  // poller返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;      // 主要作用，当mainliio获取一个新用户的channel,通过轮询算法选择一个subloop,通过该成员唤醒subloop
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel *currentActiveChannel_; // 用于assert

    std::vector<Functor> pendingFunctors_;      // 存储loop需要执行的全部操作
    std::mutex mutex_;                          // 保护调用vector容器的线程安全操作
};