#include "epoll_poller.h"
#include "Timestamp.h"
class Poller;
class EventLoop;



// ::epoll_create1( EPOLL_CLOEXEC )
// 保证该fd在fork子进程后执行exec时就关闭
EpollPoller::EpollPoller( EventLoop* loop ) 
  : Poller( loop ),
    epollfd_( ::epoll_create1( EPOLL_CLOEXEC ) ),
    events_( kInitEventListSize) 
{
  if( epollfd_ < 0 ) {
    CRITICAL( "EpollPoller::EpollPoller" );
  }
}

EpollPoller::~EpollPoller() { ::close( epollfd_ ); }


Timestamp EpollPoller::Poll( int timeout_ms, ChannelList* active_channels ) {
  // channels_.size() 构造时候初始是16，后面会动态扩容？！
  TRACE( "fd total count:{}",channels_.size() );
  int num_events = ::epoll_wait( epollfd_,
                                &*events_.begin(),
                                static_cast<int>(events_.size()),
                                timeout_ms );
  int saved_errno = errno;
  Timestamp now { Timestamp::Now() };

  // 处理事件
  if( num_events > 0 ) {
    TRACE( "{} envents happened", num_events );  
  }


}


Poller* Poller::NewPoller( EventLoop* loop ) {
  return new EpollPoller( loop );
}