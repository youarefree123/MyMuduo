#include <string>
#include <cassert>

#include "net/event_loop.h"
#include "net/eventloop_thread.h"
#include "net/eventloop_thread_pool.h"


EventLoopThreadPool::EventLoopThreadPool( EventLoop* base_loop, const std::string& name_arg ) 
 :  base_loop_( base_loop ),
    name_ ( name_arg ),
    started_( false ),
    num_threads_( 0 ),
    next_( 0 )
{
}    

/*pool析构不需要释放vector中的指针对应内存，所有loop都是栈上开辟*/
EventLoopThreadPool::~EventLoopThreadPool() {} 

void EventLoopThreadPool::Start( const ThreadInitCallback& cb ) {
    assert( !started() );
    base_loop_->AssertInLoopThread();

    started_ = true;

    // 如果只有一个loop,就永远不会进入这个for循环
    for( int i = 0; i < num_threads_; ++i ) {
        char buf[ name_.size() + 32 ] = {0};
        // 设置新name, append一下编号
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        
        EventLoopThread* t = new EventLoopThread( cb, buf );
        threads_.push_back( std::unique_ptr<EventLoopThread>( t ) );
        loops_.push_back( t->StartLoop() );
    }

    // 只有一个loop ，并且注册了回调，才会进入下面逻辑
    if( num_threads_ == 0 && cb ) {
        cb( base_loop_ );
    }


}


// 轮询,在Start()之后才会被调用
EventLoop* EventLoopThreadPool::GetNextLoop() {
    base_loop_->AssertInLoopThread();
    assert( started_ );
    EventLoop* loop = base_loop_;

    if( !loops_.empty() ) {
        // 轮询
        loop = loops_[ next_ ];
        ++next_;
        if( next_ >= loops_.size() ) {
            next_ = 0;
        }
    }
    return loop;
}