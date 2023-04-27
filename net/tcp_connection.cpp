#include <functional>
#include <cassert>
#include <memory>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <errno.h>

#include "log.h"
#include "tcp_connection.h"
#include "callbacks.h"
#include "socket_wrapper.h"
#include "event_loop.h"
#include "unlimited_buffer.h"
#include "channel.h"

// 和tcpserver的冗余了，能不能合并，在哪合并
static inline EventLoop* CHECK_NOTNULL( EventLoop* loop ) {
    if( loop == nullptr ) {
        CRITICAL("CHECK_NOTNULL failed.");
    }
    return loop;
}

TcpConnection::TcpConnection( EventLoop* loop,
                            const std::string name,
                            int sockfd,
                            const InetAddress& local_addr,
                            const InetAddress& peer_addr )
 :  loop_( CHECK_NOTNULL( loop ) ),
    name_( name ),
    state_( kConnecting ),
    reading_( true ),
    socket_( new SocketWrapper(sockfd) ),
    channel_( new Channel( loop, sockfd ) ),
    local_addr_( local_addr ),
    peer_addr_( peer_addr ),
    high_water_mark_( 64*1024*1024 )
{
    // channel 布置Pooler通知后执行的回调函数
    channel_->set_read_callback(
        std::bind( &TcpConnection::HandleRead, this, std::placeholders::_1 )
    );
    channel_->set_write_callback(
        std::bind( &TcpConnection::HandleWrite, this )
    );
    channel_->set_close_callback(
        std::bind( &TcpConnection::HandleClose, this )
    );
    channel_->set_error_callback(
        std::bind( &TcpConnection::HandleError, this )
    );

    DEBUG( "name = {}, fd = {}", name_, sockfd );
    socket_->set_keep_alive( true );
}

TcpConnection::~TcpConnection() {
    DEBUG( "name = {}, fd = {}, state = {}",name_, channel_->fd(), state_ );
    assert( state_ == kDisconnected );
}

void TcpConnection::HandleRead( Timestamp receive_time ) {
    loop_->AssertInLoopThread();
    ssize_t n = input_buffer_.ReadFd( channel_->fd() );
    if( n > 0 ) {
        // 已建立连接的用户，有可读事件发生后，需要去读一下buffer
        msg_cb_( shared_from_this(), &input_buffer_, receive_time );
    }
    else if( n == 0 ) {
        HandleClose();
    }
    else {
        ERROR( "TcpConnection::HandleRead" );
        HandleError();
    }
}

void TcpConnection::HandleWrite() {
    loop_->AssertInLoopThread();
    if( channel_->IsWriting() ) {
        ssize_t n = output_buffer_.WriteFd( channel_->fd() );  

        if( n > 0 ) {
            output_buffer_.Retrieve(n); 
            // 如果已经读完了，channel 尽快注销写实现以免Poller频繁触发
            if( output_buffer_.ReadableBytes() == 0 ) {
                channel_->DisableWriting();
                // 如果有写完成回调，执行
                if( written_cb_ ) {
                    // 这里RunInLoop不就行了？
                    // loop_->QueueInLoop(
                    //     std::bind( written_cb_, std::shared_from_this() )
                    // );
                    loop_->RunInLoop(
                        std::bind( written_cb_, shared_from_this() )
                    );
                }
                
                // 执行完写完成事件后，如果状态是正在关闭，说明此时需要关闭该连接
                if( state_ == kDisconnecting ) {
                    ShutdownInLoop();
                }  
            }
        }
        // 如果没读到数据
        else {
            ERROR("HandleWrite ERROR, no data is wrriten");
        }
    }
    else {
        // 按道理channel_此时一定可以写，所以至少应该是个ERROR吧
        ERROR( "HandleWrite ERROR, fd can not written, fd = {}",channel_->fd() );
    }

}


void TcpConnection::HandleClose() {
    loop_->AssertInLoopThread();
    TRACE( "fd = {}, state = {}", channel_->fd(), state_ );
    assert( state_ == kConnected || state_ == kDisconnecting ); /* 只有这两个状态才能调用该函数 */
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    set_state(kDisconnected);
    channel_->DisableAll();

    // 这里的逻辑有问题，一是没有判断cb有没有注册，另外没有必要两个回调分开执行，一个函数就可以
    // 另外为啥要guard_this,还不太清楚
    TcpConnectionPtr guard_this( shared_from_this() );
    conn_cb_( guard_this ); /* 用户注册的OnConnection 包含连接成功，建立，断开，所以在即将断开时需要执行对应回调.( 但是为啥需要guard_this？ ) */

    // must be the last line
    close_cb_( guard_this );
}


void TcpConnection::HandleError() {
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if( ::getsockopt( channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen ) < 0 ) {
        err = errno;
    }
    else {
        err = optval;
    }
    ERROR( "TcpConnection::HandleError" );
}


// c++ 11
void TcpConnection::send( string&& message ) {
    if( state_ == kConnected ) {
        // 确认是否在同一线程
        if( loop_->IsInLoopThread() ) {
            
        }


    }
    else {
        ERROR( "TcpConnection::send" );
    }
} 
// void send(UnlimitedBuffer&& message) {  // C++11
//     if( state_ == kConnected )
// }
// void TcpConnection::Send( const void* message, int len ){} /* C++11是不是可以优化？ */





void TcpConnection::Shutdown() {
    if( state_ == kConnected ) {
        set_state( kDisconnecting );
        loop_->RunInLoop( 
            std::bind( &TcpConnection::ShutdownInLoop, this )
        );
    }
} 

void TcpConnection::ShutdownInLoop() {
    loop_->AssertInLoopThread();
    if( !channel_->IsWriting() ) {
        socket_->ShutdownWrite();
    }
}

void TcpConnection::ConnectEstablished() {
    loop_->AssertInLoopThread();
    assert( state_ == kConnecting );
    set_state( kConnected );
    channel_->Tie( shared_from_this() ); /* 为什么要用shared_from_this参见头文件介绍 */
    channel_->EnableReading();

    conn_cb_( shared_from_this() );
} 

void TcpConnection::ConnectDestroyed() {
    loop_->AssertInLoopThread();
    if( state_ == kConnected ) {
        set_state( kDisconnected );
        channel_->DisableAll(); 

        conn_cb_( shared_from_this() );
    }
    channel_->Remove();
}

