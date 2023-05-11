#include <algorithm>
#include <signal.h>
#include <unistd.h>
#include <functional>
#include <sys/eventfd.h>

#include "log.h"
#include "net/poller.h"
#include "net/channel.h"
#include "net/event_loop.h"
#include "net/base/current_thread.h"
namespace {

// __thread 也就是thread_local 
// 防止一个线程创建多个eventloop
__thread EventLoop* t_loop_in_this_thread = nullptr;

// 超时时间
const int KPollTimeMs = 10000;

// 创建wakeup_fd
int CreatEventFd() {
    // CLOEXEC 在fork的子进程中用exec系列系统调用加载新的可执行程序之前，关闭子进程中fork得到的fd。
    int evtfd = ::eventfd( 0, EFD_NONBLOCK | EFD_CLOEXEC );
    if( evtfd < 0 ) {
        CRITICAL( "Failed in eventfd" );
    }
    return evtfd;
}

// (忽略信号的？？)不知道干啥的，先拷贝过来
#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;

}  // namespace



/*
    EventLoop 类函数
*/

EventLoop* EventLoop::GetEventLoopOfCurrentThread() {
    return t_loop_in_this_thread;
}

EventLoop::EventLoop() 
  : looping_( false ),
    quit_( false ),
    event_handling_( false ),
    calling_pending_functors_( false ),
    thread_id_( CurrentThread::Tid() ),
    p_poller_( Poller::NewPoller( this ) ),
    wakeup_fd_( CreatEventFd() ),
    p_wakeup_channel_( new Channel( this, wakeup_fd_ ) ),
    current_active_channel_( nullptr )
{
    DEBUG( "EventLoop created:{},tid:{}",reinterpret_cast<size_t>(this),thread_id_ );
    if( t_loop_in_this_thread ) {
        CRITICAL( "Another EventLoop {} exists in this thread", reinterpret_cast<size_t>(t_loop_in_this_thread) );
    } 
    else {
        t_loop_in_this_thread = this;
    }

    // 注册wakeup_fd的感兴趣事件以及相应的回调
    p_wakeup_channel_->set_read_callback( std::bind( &EventLoop::HandleRead, this ) );
    // 每个eventloop都会监听EventChannel的EPOLLIN事件
    p_wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
    // wakeup 清空事件，并从loop中移除
    DEBUG( "EventLoop::~EventLoop(),eventloop:{},tid:{}",reinterpret_cast<size_t>(this),thread_id_ );
    p_wakeup_channel_->DisableAll();  
    p_wakeup_channel_->Remove();  
    ::close( wakeup_fd_ );
    t_loop_in_this_thread = nullptr;
}

/**
 *  WakeUp 往wakeupfd中塞八个字节，用于结束poll的阻塞
 *  一个loop都有对应的自己的wakeupfd， WakeUp这个函数是给其他loop调用的，用于唤醒本loop
 * */
void EventLoop::WakeUp() {
    size_t one = 1;
    ssize_t n = ::write(wakeup_fd_, &one, sizeof(one) );
    if( n != sizeof(one) ) {
        ERROR( "EventLoop::WakeUp()" );
    }
}

/**
 * wakeupfd在epoll上注册读事件，对应的回调就是HandleRead()函数
 * */
void EventLoop::HandleRead() {
    size_t one = 1;
    ssize_t n = ::read( wakeup_fd_, &one, sizeof(one) );
    if( n != sizeof(one) ) {
        ERROR( "EventLoop::HandleRead()" );
    }
}

/**
 *  开启轮训：
 *  1、Poll 阻塞等待事件发生
 *  2、遍历所有发生事件的channel，并执行对应回调
 *  3、执行Functor队列中的回调
 * 
 *  为什么要3:DoPendingFunctors() ? 
 *  mainloop负责accept和唤醒subloop
 *  唤醒方式就是利用eventfd，触发一个读事件，subloop就会从poll函数处唤醒，然后执行被manloop塞过来的回调，这些回调都在Functor队列中
 **/
void EventLoop::Loop() {
    AssertInLoopThread();
    looping_ = true;
    quit_ = false;
    TRACE( "EventLopp {} start looping!",reinterpret_cast<size_t>(this) );

    while( !quit_ ) {
        // 清空原来的活跃Channel, clear 让size = 1;
        active_channels_.clear();
        // 主loop会监听client的fd和wakeup的fd
        // 那subloop呢？
        poll_return_time_ = p_poller_->Poll( KPollTimeMs, &active_channels_ );

        for(Channel* current_channel : active_channels_) {
            // 处理对应事件
            current_channel->HandleEvent( poll_return_time_ );
        }
        /**
         *  主eventloop主要处理accept和wakeup，是在上面轮训中处理的，从eventloop Poll只会触发wakeup，然后从poll函数那唤醒，上面的轮训只会执行wakeupfd的读字节操作，真正处理回调是在下面的函数中 
         **/
        DoPendingFunctors();
    }

    looping_ = false;
}
/** 
 退出事件循环,可能在某个回调中调用？？
    1、loop在自己的线程中调用，quit_变为true，然后退出loop
    2、如果是本线程中调用其他subloop的Quit(),
 **/
void EventLoop::Quit() {
    // quit_ 需要是原子的，否则可能会出现并发问题
    // 例如，quit_ 为false的时候，执行到while(!quit_), 进入循环后，quit_ = true了，然后执行析构，最后再执行循环后的语句，出现访问无效对象的问题？？（为什么不在析构做断言？？） 
    quit_  = true; 

    // 如果不是本线程的loop，其loop的quit_改成true也不会立即生效，因为可能该loop还在Poll那卡着，所以需要唤醒一下该loop。
    if( !IsInLoopThread() ) {
        WakeUp();
    }

}
/*
    只有该channel是我该loop的，并且该loop也是我当前线程的loop，才会去更新channel
    实际最后会调用Poller的Update，EventLoop不关心channel如何更新
*/


void EventLoop::UpdateChannel( Channel* channel ) {
    assert( channel->owner_loop() == this );
    AssertInLoopThread();
    p_poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel( Channel* channel ) {
    assert( channel->owner_loop() == this );
    AssertInLoopThread();
    // 这个断言是干啥的
    if( event_handling_ ) {
        assert( current_active_channel_ == channel || 
                std::find( active_channels_.begin(), active_channels_.end(), channel ) == active_channels_.end() );
    }
    p_poller_->RemoveChannel( channel );
}   

/**
 * 返回是否是本Loop的channel， 实际上还是调用Poller的HasChannel来判断
*/
bool EventLoop::HasChannel( Channel* channel ) {
    assert( channel->owner_loop() == this );
    AssertInLoopThread();
    return p_poller_->HasChannel( channel );
}

void EventLoop::AbortNotInLoopThread() {
    CRITICAL( "EventLoop::AbortNotInLoopThread EventLoop {} was created in threadId_ {}, current_id = {}",
               reinterpret_cast<size_t>(this), thread_id_, CurrentThread::Tid() );
}

/**
 * 执行本loop中回调队列里的所有任务
*/
void EventLoop::DoPendingFunctors() {
    std::vector<Functor> functors;
    calling_pending_functors_ = true;

    // 利用局部变量，交换两个容器
    // 减少临界区长度（减少mainloop塞回调的时延）、避免死锁（functor可能会再调用QueueInLoop,会有同一线程加两次锁的问题）
    {
        std::lock_guard<std::mutex> lock( mtx_ );
        functors.swap( pending_functors_ );  
    }

    for(auto& functor: functors) {
        functor();
    }

    calling_pending_functors_ = false;
}

/**
 *   在该loop中执行cb
 *   如果是非本loop线程 调用的其他线程loop的该函数，不能立即执行，需要将cb放入队列
 *   然后唤醒对应的worker线程来在该loop中执行
 *  （为了避免线程安全问题，每个线程和loop是一一绑定的） 
 * */
void EventLoop::RunInLoop( Functor cb ) {
    if( IsInLoopThread() ) {
        DEBUG( "EventLoop {} , threadId_ {}, current_id = {}",
               reinterpret_cast<size_t>(this), thread_id_, CurrentThread::Tid() );
        cb();
    }
    else {
        QueueInLoop( std::move( cb ) );
    }
}

void EventLoop::QueueInLoop( Functor cb ) {
    // 因为可能有不只一个其他loop在同一时间往该loop队列里append回调，所以该队列是临界区
    {
        std::lock_guard<std::mutex> lock( mtx_ );
        pending_functors_.push_back( std::move( cb ) );
    }

    // 如果是非本线程访问该loop，或是在本线程，目前正在执行 pending_functors_ 中的回调
    // 换句话说，只有在loop函数（134行），执行本active_channels_中的cb时，不触发wakeup
    // （因为此时还没有执行DoPendingFunctors，容器还没有交换，此时append进去的回调在本轮就会直接执行）
    if( !IsInLoopThread() || calling_pending_functors_ ) {
        WakeUp();
    }
}