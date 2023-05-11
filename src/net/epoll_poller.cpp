
#include <strings.h>
#include <assert.h>
#include <sys/epoll.h>

#include "log.h"
#include "net/channel.h"
#include "net/poller.h"
#include "net/epoll_poller.h"
#include "net/base/timestamp.h"


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

EpollPoller::~EpollPoller(){ ::close( epollfd_ ); }


Timestamp EpollPoller::Poll( int timeout_ms, ChannelList* active_channels ) {
    // events_list_.size() 构造时候初始是16，后面会动态扩容？！
    TRACE( "fd total count:{}",channels_.size() );
    int num_events = ::epoll_wait( epollfd_,
                                    &*events_list_.begin(),
                                    static_cast<int>(events_list_.size()),
                                    timeout_ms );
    int saved_errno = errno;
    Timestamp now { Timestamp::Now() };

    // 处理事件
    if( num_events > 0 ) {
        TRACE( "{} envents happened", num_events );  
        FillActiveChannels( num_events, active_channels ); /* 根据拿到的events_list_ */
        //  自适应，如果一次没有取完，会自动扩容
        if( static_cast<size_t>( num_events ) == events_list_.size() ) {
        events_list_.resize( events_list_.size() * 2 );
        }
    }
    else if( num_events == 0 ) {
        TRACE( "Nothing happened." )
    }
    else {
        // 如果不是被信号打断的，还进入了这个作用域，就说出出错了
        if( saved_errno != EINTR ) {
        errno = saved_errno;
        ERROR( "EpollPoller::Poll" );
        }
    }
    return now;

}

/*
  根据当前channel 维护的fd的状态，更新该fd监听的事件
  fd的之前状态不同，操作也不同
*/
void EpollPoller::UpdateChannel( Channel* channel ) {
    const int fd_status = channel->fd_status();
    // TRACE( "Before UpdateChannel : fd = {}, events = {}, fd_status = {}", \
    //         channel->fd(), channel->events(), channel->fd_status() );
    int fd = channel->fd();
    // 未监听fd，或者是已删除监听的fd
    // 问题：如果我KNew和KDeleted的Channel add一个空事件会怎么样？
    if( fd_status == KNew || fd_status == KDeleted ) {
        if( fd_status == KNew ) {
        assert( channels_.find( fd ) == channels_.end() );
        channels_[fd] = channel;
        } 
        else {
        assert( channels_.find( fd ) != channels_.end() );
        assert ( channels_[fd] = channel );
        }

        channel->set_fd_status( KAdded );
        Update( EPOLL_CTL_ADD, channel );
    }
    // 正常更新，可能会有删除操作
    else {
        assert( channels_.find( fd ) != channels_.end() );
        assert( channels_[fd] = channel );
        assert( fd_status == KAdded );

        if( channel->IsNoneEvent() ) {
        Update( EPOLL_CTL_DEL, channel );
        channel->set_fd_status( KDeleted );
        }
        else {
        Update( EPOLL_CTL_MOD, channel );
        }

    }

    TRACE( "After UpdateChannel : fd = {}, events = {}, fd_status = {}, loop = {}", \
            channel->fd(), channel->events(), channel->fd_status(), reinterpret_cast<size_t>( channel->owner_loop() )  );
} 

/**
 *  如果channel 此时监听空事件，就从ChannleMap中移除该channel
 * 删除包含三件事
 * 1、将该channel 从 channels_ 中移除
 * 2、如果该channel还是Kadded状态,则还需要从epoll_fd的监听队列中移除
 * 3、将该channel的状态设置成kNew
*/
void EpollPoller::RemoveChannel( Channel* channel ) {
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->IsNoneEvent());

    int fd_status = channel->fd_status();
    assert( fd_status == KAdded || fd_status == KDeleted );
    size_t num = channels_.erase( fd ); 
    assert( num == 1 );

    
    if( fd_status == KAdded ) {
        Update( EPOLL_CTL_DEL, channel );
    }
    channel->set_fd_status( KNew );  
    TRACE( "After RemoveChannel fd = {}, events = {}, fd_status = {}", \
            channel->fd(), channel->events(), channel->fd_status() );
}

// 像epollfd注册该channel的对应感兴趣事件
void EpollPoller::Update( int operation, Channel* channel) {
    struct epoll_event event;
    bzero( &event, sizeof(event) );
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    TRACE( "EPOLL_CTL op = {}, fd = {}, event = {}",
            OperationToString(operation),
            channel->fd(), 
            channel->EventsToString( channel->events() ) );

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


// 将所有的epoll_event 都转回 channel
void EpollPoller::FillActiveChannels( int num_events, ChannelList* active_channels ) const {
    assert( num_events <= events_list_.size() );
    for( int i = 0; i < num_events; i++ ) {
        Channel* channel = static_cast< Channel* > ( events_list_[i].data.ptr );
        int fd = channel->fd();
        assert( channels_.find( fd ) != channels_.end() );

        channel->set_revents( events_list_[i].events );
        active_channels->push_back(channel); 
    }
}


Poller* Poller::NewPoller( EventLoop* loop ) {
    return new EpollPoller( loop );
}
