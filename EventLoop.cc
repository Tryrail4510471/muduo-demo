#include "EventLoop.h"
#include "Poller.h"
#include "Logger.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>

// 防止一个线程创建多个EventLoop  thread_local
__thread EventLoop *t_loopInThisThread = nullptr;

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// 创建wakeupfd ，用俩notify唤醒subRector处理心来的channel
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("eventfd error:%d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , callingPendingFunctors_(false)
    . threadId_(CurrentThread::tid())
    , poller_(Poller::newDefaultPoller(this))
    , wakeupfd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupfd_))
    , currentActiveChannel_(nullptr)
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_)
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread%d \n", t_loopInThisThread. threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    // 设置wakeupfd事件的类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个eventloop都将监听wakeupChannel的EPOLLIN读事件了
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::clase(wakeupFd_);
    t_loopInThisThread = nullptr;
}

// 开启事件循环
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping \n", this);

    while(!quit)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (Channel *channel : activeChannels_)
        {
            // Poller监听哪些channel发生事件了， 然后上报给EventLoop,通知channel处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }
        // 执行当前EventLoop事件循环需要处理的回调操作
        /**
         * IO线程 mainLoop accept fd 《= channel subloop
         * mainLoop事先注册一个回调cb （需要subloop来执行） wakeup subloop后执行下面的回调操作
        */
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping \n", this);
    looping_ = false;
} 

// 退出事件循环     1：loop在自己的线程中调用quit   2：在非loop的线程中，调用loop的quit
void EventLoop::quit()
{
    quit_ = true;

    if (!isInLoopThisThread())  // 如果在其他线程中，调用的quit 在一个sublooop中调用了mainloop的quit
    {
        wakeup();
    }
}

// 在当前loop中执行cb
void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())   // 在当前的loop线程中执行cb （callback）
    {
        cb();
    }
    else    // 在非当前线程中执行cb,需要唤醒loop所在线程， 执行cb
    {
        queueInLoop(cb);
    }
}
// 吧cb放入队列中，唤醒loop「所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_looc<std::mutrex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    // 唤醒相应的，需要执行上面回调操作的loop的线程了
    // callingPendingFunctors_： 当前loop正在执行回调， 但是loop又有了新的回调
    if (!isInLoopThread() || callingPendingFunctors_) // ?callingPendingFunctors_
    {
        wakeup();   // 唤醒loop所在的线程
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8", n);
    }
}

// 用来唤醒loop所在的线程 向wakeupfd_写一个数据
viud EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOS_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n)
    }
}

// EventLoop的方法 ==》 Poller的方法
void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel *channel)
{
    poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() // 执行回调
{
    std::vector<Functor> functors;  // 临时的vector对象保存要回调的事件队列，避免长时间占用对象，提高并发性
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex>(mutex_);
        functors.swap(pendingFunctors_);
    }
    
    // 调用相应回调
    for (const Functor& functor : funtors)
    {
        functor();  // 执行当前loop需要执行的回调操作
    }

    callingPendingFunctors_  = false;
}