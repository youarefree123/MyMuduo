#pragma once 

#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

#include "net/base/current_thread.h"
#include "net/base/timestamp.h"
#include "net/base/noncopyable.h"


class Channel;
class Poller;


/**
 *  muduo 没有使用常见的生产者-消费者的线程安全队列来作为mainloop和subloop的通信方式
 *  而是使用eventfd，mainloop通过轮询的方式分发fd（channel）给subloop
*/


class EventLoop : noncopyable
{

public: 
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    // 事件循环
    void Loop(); 
    // 关闭事件循环
    void Quit();

    // 轮询返回的时间，通常意味着有事件到达
    Timestamp PollReturnTime() const { 
        return poll_return_time_;
    }
    // 立即执行一个回调（在当前线程中）
    // 在其他线程中使用是安全的
    void RunInLoop( Functor cb );
    // 把上层注册的回调函数放入队列，并唤醒loop所在线程来执行
    void QueueInLoop( Functor cb );

    // 内部使用，用来唤醒loop所在的线程
    void WakeUp();
    void UpdateChannel( Channel* channel );
    void RemoveChannel( Channel* channel );
    bool HasChannel( Channel* channel );

    bool IsInLoopThread() const {
        return thread_id_ == CurrentThread::Tid();
    }
    // 断言，确认loop是否在对应线程里
    void AssertInLoopThread() {
        if( !IsInLoopThread() ) {
            AbortNotInLoopThread();
        }
    }
    bool EventHandling() const {
        return event_handling_;
    }
    // 为什么要设置成静态的?
    static EventLoop* GetEventLoopOfCurrentThread();

private:
    using ChannelList = std::vector<Channel*>;

    void AbortNotInLoopThread(); 
    void HandleRead(); /* event_fd读取内容，唤醒睡眠的loop */ 
    void DoPendingFunctors(); 
   


    bool looping_; /* atomic , 是否在循环*/ 
    std::atomic<bool> quit_;  /*是否已经终止循环*/
    bool event_handling_; /* atomic, 是否有事件正在处理 */
    bool calling_pending_functors_; /* atomic 是否正在调用回调？？ */
    const pid_t thread_id_; /* 当前loop的线程 */
    Timestamp poll_return_time_; /* 记录本次poll的返回时间点 */
    std::unique_ptr<Poller> p_poller_; /*  */ 
    int wakeup_fd_; /* 重要，唤醒subloop 的fd*/
    std::unique_ptr<Channel> p_wakeup_channel_;  

    // 暂存变量
    ChannelList active_channels_; 
    Channel* current_active_channel_;

    std::mutex mtx_;
    std::vector<Functor> pending_functors_; /* 待执行的回调 */
};

