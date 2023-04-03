#include "epoll_poller.h"
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
  // channels_.size()确定是监听的fd总数吗？如果没删，只置空怎么办
  TRACE( "fd total count:{}",channels_.size() );
  int num_events = ::epoll_wait(  )
}


Poller* Poller::NewPoller( EventLoop* loop ) {
  return new EpollPoller( loop );
}