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

    close_cb_( guard_this );
    // must be the last line
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



void TcpConnection::Send( const string& buf ) {
    if( state_ == kConnected ) {
        // 确认是否在同一线程,是的话直接SendInLoop
        if( loop_->IsInLoopThread() ) {
            DEBUG( "loop_->IsInLoopThread(), buf = {}", buf );  
            SendInLoop( buf.c_str(), buf.size() );
        }
        else {
            // 这里RunInLoop中一定调用的是QueueInLoop吧，因为此时thread和loop不是对应的
            loop_->RunInLoop(
                std::bind( &TcpConnection::SendInLoop, this, buf.c_str(), buf.size() )
            );
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

/** 
 *  conn 发送数据时，应用发送过快，超过内核协议栈发送速度，则应用需要先将待发送数据写入buffer中，用以缓冲
 *  设置高水位回调，动态调节应用发送速度
*/
void TcpConnection::SendInLoop(const void* message, size_t len) {
    ssize_t nwrote = 0; /* 已写数 */
    size_t remaining = len; /* 剩余数 */
    bool fault_error = false;

    // 之前调用过conn的shutdown,写端关闭，那么就不能发送了，出现错误
    if( state_ == kDisconnected ) {
         CRITICAL( "disconnected give up writing!" );
    }

   

    // 如果channel_ 第一次开始写数据，并且缓冲区中没有待发送数据，先去尽力写一次，能一次写完就不用设置Epoll写事件了
    if( !channel_->IsWriting() && output_buffer_.ReadableBytes() == 0 ) {
        nwrote = ::write( channel_->fd(), message, len );
        DEBUG(" SendInLoop written {} byte ", nwrote );
        if( nwrote < 0 ) { // 错误处理, 参照原有的
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) { // SIGPIPE  RESET
                    fault_error = true;
                }
            }
        }
        else { // nwrote >= 0
            remaining = len - nwrote;
            if( remaining == 0 && written_cb_ ) {
                // 内核协议栈缓存支持一次write就写完，就i不用再给channel设置epollout事件了
                // 如果设置了写完成事件,还需要执行写完成回调(低水位回调)
                /* 这里为什么是QueueInLoop?? */
                loop_->QueueInLoop( 
                    std::bind( written_cb_, shared_from_this() )
                 );
            }
        }

    }

    // 如果一次write没有读完，剩余的数据需要保存到缓冲区当中，然后给channel
    // 注册epollout事件，poller发现tcp的发送缓冲区有空间，会通知相应的sock-channel，调用writeCallback_回调方法
    // 也就是调用TcpConnection::handleWrite方法，把发送缓冲区中的数据全部发送完成

    if( !fault_error && remaining > 0 ) {
        size_t old_len = output_buffer_.ReadableBytes(); /* buffer里的待发送数据 */
        
        // 如果触发了高水位（高水位的触发一定是原buffer里水位没触发，加了old_len后超过，并且设置了高水位触发回调）, 执行高水位回调
        // TcpServer 里好像没有设置该回调的函数啊。后期加一下 
        if( old_len + remaining >= high_water_mark_ && 
            old_len < high_water_mark_ &&
            high_water_mark_cb_ )
        {
            // 这里为什么也是QueueInLoop?
            loop_->QueueInLoop(
                std::bind( high_water_mark_cb_, shared_from_this(), remaining )
            );
        }
        // 将剩余的数据添加进outbuffer,待内核缓冲区有空闲时再进行写操作
        output_buffer_.Append( (char*)message + nwrote, remaining );

        if( !channel_->IsWriting() ) {
            channel_->EnableWriting(); /* 如果Epoll上没有注册写事件，需要注册写事件 */
        }

    }
   
}


void TcpConnection::Shutdown() {
    TRACE(" Shutdown: state_ = {} ",state_);
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
    // 为什么要Tie？
    // 防止用户如果手动Remove了这个Conn的时候，Conn被释放，Channel_ 对应的Conn的操作都会造成未知错误
    // 所以干脆直接Tie一下，保证Channel的生命周期内conn一定没有被释放？！
    channel_->Tie( shared_from_this() ); /* 为什么要用shared_from_this参见头文件介绍 */
    channel_->EnableReading();

    conn_cb_( shared_from_this() );
} 


void TcpConnection::ConnectDestroyed() {
    loop_->AssertInLoopThread();
    if( state_ == kConnected ) {
        set_state( kDisconnected );
        channel_->DisableAll(); /* 把channle所有感兴趣事件全部移除 */

        conn_cb_( shared_from_this() ); /*  */
    }
    channel_->Remove();
}

