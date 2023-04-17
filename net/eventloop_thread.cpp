#include <functional>
#include <cassert>

#include "eventloop_thread.h"
#include "event_loop.h"



/**
 * thread_ 调用ThreadWrapper类的构造，生成一个
*/
EventLoopThread::EventLoopThread( const ThreadInitCallback& cb, const std::string& name ) 
  : loop_( nullptr ),
    exiting_( false ),
    thread_( std::bind( &EventLoopThread::thread_func, this ), name ),
    mtx_(), 
    cond_(),
    cb_(cb) 
{
}

// 不是完全懂这里的竞争问题。
EventLoopThread::~EventLoopThread() {
    assert( !exiting_ );
    exiting_ = true; 
     // not 100% race-free, eg. threadFunc could be running callback_.
    if( loop_ != nullptr ) { 
        loop_->Quit();
        thread_.Join();
    }
}


/**
 *  只有线程开启后才能开启循环
*/
EventLoop* EventLoopThread::StartLoop() {
    assert( !thread_.started() );
    thread_.Start();
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mtx_);
        // 使用while防止虚假唤醒
        while( loop_ == nullptr ) {
            cond_.wait( lock );
        }
        // // 或者使用lambda, 条件满足就一直wait
        // cond_.wait( lock, [](){ return loop_ != nullptr; } )
        loop = loop_;
    }
    return loop;
}


 /**
  * 每个thread执行的func
 */
void EventLoopThread::thread_func() {
    EventLoop loop{};
    // 如果设置了对loop做初始化，就去做一下
    if( cb_ ) {
        cb_( &loop );
    }

    // 绑定线程唯一的loop_
    {
        std::lock_guard<std::mutex> lock( mtx_ );
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.Loop(); /*开启loop*/

    /*结束loop,本thread绑定的loop_需要制空（涉及竞争？！）*/
    std::lock_guard< std::mutex > lock( mtx_ );
    loop_ = nullptr;
}

