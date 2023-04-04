#include <strings.h>
#include <assert.h>
#include <sys/epoll.h>

#include "epoll_poller.h"
#include "poller.h"
#include "timestamp.h"
#include "log.h"

static const int KNew = -1;     // fd 未被监听
static const int KAdded = 1;    // fd 已监听
static const int KDeleted = 2;  // fd 被移除监听

// ::epoll_create1( EPOLL_CLOEXEC )
// 保证该fd在fork子进程后执行exec时就关闭
EpollPoller::EpollPoller( EventLoop* loop ) 
  : Poller( loop ),
    epollfd_( ::epoll_create1( EPOLL_CLOEXEC ) ),
    events_list_( kInitEventListSize) 
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
                                &*events_list_.begin(),
                                static_cast<int>(events_.size()),
                                timeout_ms );
  int saved_errno = errno;
  Timestamp now { Timestamp::Now() };

  // 处理事件
  if( num_events > 0 ) {
    TRACE( "{} envents happened", num_events );  
  }


}

/*
  根据当前channel 维护的fd的状态，更新该fd监听的事件
  fd的之前状态不同，操作也不同
*/
void EpollPoller::UpdateChannel( Channel* channel ) {
  const int fd_status = channel->fd_status();
  TRACE( "fd = {}, events = {}, fd_status = {}", \
          channel->fd(), channel->events(), channel->fd_status() );
  int fd = channel->fd();
  // 未监听fd，或者是已删除监听的fd
  // 问题：如果我add一个空事件会怎么样？
  if( fd_status == KNew || fd_status == KDeleted ) {
    if( fd_status == KNew ) {
      assert( channels_.find( fd ) == channels_.end() );
      channels_[fd] = channel;
    } 
    else {
      assert( channels_.find( fd ) != channels_.end() );
      assert ( channels_[fd] = channel );
    }

    channel.set_fd_status( KAdded );
    Update( EPOLL_CTL_ADD, channel );
  }
  // 正常更新，可能会有删除操作
  else {
    assert( channel_s.find( fd ) != channels_.end() );
    assert ( channels_[fd] = channel );
    assert( fd_status == KAdded );

    if( channel->IsNoneEvent() ) {
      Update( EPOLL_CTL_DEL, channel );
      channel->set_fd_status( KDeleted );
    }
    else {
      Update( EPOLL_CTL_MOD, channel );
    }

  }

} 

/**
 *  如果channel 此时监听空事件，就从ChannleMap中移除该channel
*/
void EpollPoller::RemoveChannel( Channel* channel ) {
  int fd = channel->fd();
  TRACE( "fd = {}", fd );
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());

  int fd_status = channel->fd_status();
  assert( fd_status == KAdded || fd_status == KDeleted );
  size_t num = channels_.erase( fd ); 
  assert( n == 1 );

  if( fd_status == KAdded ) {
    Update( EPOLL_CTL_DEL, channel );
  }
  channel->set_fd_status( KNew );  
}

void EpollPoller::Update( int operation, Channel* channel) {
  struct epoll_event event;
  bzero( &event, sizeof(event) );
  event.events = channle->events();
  event.data.ptr = channel;
  int fd = channel.fd();
  TRACE( "EPOLL_CTL op = {}, fd = {}, event = {}",\
          OperationToString(operation),\  
          channel->fd(), \ 
          EventsToString( channel->events() ) );

  if( ::epoll_ctl( epollfd_, operation, fd, &event ) < 0 ) {
    CRITICAL( "EpollPoller::Update( int operation, Channel* channel)" );
  }

} 


const char* EpollPoller::OperationToString( int op ) const{
  switch (op)
  {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}


Poller* Poller::NewPoller( EventLoop* loop ) {
  return new EpollPoller( loop );
}