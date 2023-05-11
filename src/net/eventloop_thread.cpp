#include <functional>
#include <cassert>

#include "net/eventloop_thread.h"
#include "net/event_loop.h"



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
        // 使用while防止虚假唤醒,Start()以后可能还没分配到一个loop，所以需要wait来同步
        while( loop_ == nullptr ) {
            cond_.wait( lock );
        }
        // // 或者使用lambda, 条件满足就一直wait
        // cond_.wait( lock, [](){ return loop_ != nullptr; } )
        loop = loop_; // 这个是新版新加的，以前是直接返回loop_;(为什么要这么处理)
    }
    return loop;
}


 /**
  * 线程主函数在栈上定义一个EventLoop对象,在这里绑定唯一的loop
 */
void EventLoopThread::thread_func() {
    EventLoop loop{}; /*eventloop 都是栈上对象，不需要手动释放*/
    // 如果设置了对loop一些操作，就去做一下
    if( cb_ ) {
        cb_( &loop );
    }

    // 绑定线程唯一的loop_
    {
        std::lock_guard<std::mutex> lock( mtx_ );
        loop_ = &loop;
        cond_.notify_one(); // 在临界区内唤醒，以免唤醒丢失问题
    }

    loop.Loop(); /*开启loop*/

    /*结束loop,本thread绑定的loop_需要制空（涉及竞争？！）*/
    std::lock_guard< std::mutex > lock( mtx_ );
    loop_ = nullptr;
}

