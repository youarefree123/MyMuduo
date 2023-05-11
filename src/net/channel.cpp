#include <sys/epoll.h>

#include "net/event_loop.h"
#include "net/channel.h"
#include "log.h"



const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; // 读事件 , 当有带外数据时候触发 EPOLLPRI 
const int Channel::kWriteEvent = EPOLLOUT;


Channel::Channel( EventLoop* loop, size_t fd ) 
  : loop_( loop ),
    fd_( fd ),
    events_( kNoneEvent ),
    revents_( kNoneEvent ),
    fd_status_( -1 ),
    tied_ ( false )  {}
// 原版全是判断Channel是否有回调执行和Channel是否属于当前loop的断言

// 释放对应资源（就是一堆断言，啥也没做）
// 一个Channel的声明周期到底有多长
Channel::~Channel()
{
//   assert(!event_handling_);
//   assert(!addedToLoop_);
  if (loop_->IsInLoopThread())
  {
    assert(!loop_->HasChannel(this));
  }
}


// Tie 上obj，保证资源不被释放？？这是观察者模型的应用
void Channel::Tie( const std::shared_ptr< void >& obj ) {
    tie_ = obj;
    tied_ = true;
}
// 更新fd的events, 借助loop来实现
void Channel::Update() {
// add code
    loop_->UpdateChannel(this);
} 
// 在当前loop中删除该Channel
void Channel::Remove() {
  // add code
  loop_->RemoveChannel(this);
}

// 本质就是执行回调，但是为什么需要Tie和非Tie呢？
void Channel::HandleEvent( Timestamp receive_time ){
    if( tied_ ) {
        // 提升至强智能指针，变量在栈上
        std::shared_ptr< void > guard = tie_.lock();
        if( guard ) {
        HandleEventWithGuard( receive_time );
        }
    } 
    else {
        HandleEventWithGuard( receive_time );
    }
}

void Channel::HandleEventWithGuard( Timestamp receive_time ) {
    // fd挂起(套接字已不在连接中), 并且内核缓冲区已经没有数据可读
    if( (revents_ & EPOLLHUP ) && !(revents_ & EPOLLIN ) ) {
        WARN( "fd = {}, Channel::HandleEventWithGuard() EPOLLHUP", fd_ );
        if( close_callback_ ) close_callback_();
    }

    if( revents_ & EPOLLERR ) {
        if( error_callback_ ) error_callback_();
    }

    // 读关闭事件为啥也要调用读的回调？
    if( revents_ & (kReadEvent | EPOLLRDHUP ) ) {
        TRACE( "fd = {}, Channel::HandleEventWithGuard() EPOLLIN", fd_ );
        if( read_callback_ ) read_callback_( receive_time );
    }

    if( revents_ & kWriteEvent ) {
        TRACE( "fd = {}, Channel::HandleEventWithGuard() EPOLLOUT", fd_ );
        if( write_callback_ ) write_callback_();
    }

}

string Channel::EventsToString(int ev) {
    std::ostringstream oss;
    if (ev & EPOLLIN) oss << "IN ";
    if (ev & EPOLLPRI) oss << "PRI ";
    if (ev & EPOLLOUT)  oss << "OUT ";
    if (ev & EPOLLHUP) oss << "HUP ";
    if (ev & EPOLLRDHUP) oss << "RDHUP ";
    if (ev & EPOLLERR) oss << "ERR ";
    return oss.str();
}