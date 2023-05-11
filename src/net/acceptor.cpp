#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include "log.h"
#include "net/acceptor.h"
#include "net/channel.h"
#include "net/socket_wrapper.h"
#include "net/inet_address.h"
#include "net/event_loop.h"

// 只支持网络套接字，TCP, 非阻塞， fork后子进程关闭,最后一个参数写0 也可以
static SocketWrapper CreateNonblockSocket() {
    int sockfd = ::socket( AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC , IPPROTO_TCP );
    if( sockfd < 0 ) {
        CRITICAL( "CreateNonblockFd failed." );
    }
    return SocketWrapper( sockfd );
}


Acceptor::Acceptor( EventLoop* loop, const InetAddress& listen_addr, bool reuse_port ) 
  : loop_( loop ),
    listen_socket_( CreateNonblockSocket() ),
    listen_channel_( Channel( loop_, listen_socket_.fd() ) ),
    listening_( false ),
    idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    // 监听fd的初始化，绑定
    listen_socket_.set_reuse_addr( true );
    listen_socket_.set_reuse_port( reuse_port );
    listen_socket_.BindAddress( listen_addr );

    // listenfd注册读事件
    listen_channel_.set_read_callback( std::bind( &Acceptor::HandleRead, this ) );
}

// Acceptor 都没了，程序也没必要执行下去了吧
Acceptor::~Acceptor() {
    listen_channel_.DisableAll();
    listen_channel_.Remove();
}


void Acceptor::Listen() {
    loop_->AssertInLoopThread();
    listening_ = true;
    listen_socket_.Listen(); // listenfd 是非阻塞的，那么listen会不会 阻塞在这里？？
    listen_channel_.EnableReading(); /* Listen返回说明有新连接，就需要注册读事件 */
}

/**
 * 核心函数,完成实际上的accept
*/
void Acceptor::HandleRead() {
    loop_->AssertInLoopThread();
    InetAddress peer_addr{};
    int connfd = listen_socket_.Accept( &peer_addr );

    if( connfd < 0 ) { // 未成功accept的情况，需要通过预留的fd，来处理该链接。 那如果不处理会怎么样？
        ERROR("Acceptor::HandleRead");
        // 如果文件描述符到了上限,使用预留的指向空洞文件的fd，来关闭该链接
        if( errno == EMFILE ) {
            ::close(idle_fd_);
            idle_fd_ = ::accept(listen_socket_.fd(), NULL, NULL);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
    else {
        // 对接收到的fd做初始化操作，如果没有注册过，则直接关闭
        // new_conn_cb_ 何时注册的？ TcpServer 构造的时候，设置该server对应的唯一的acceptor的new_conn_cb
        if( new_conn_cb_ ) {
            // 传递给回调的是远端的fd和其对应的addr
            new_conn_cb_( connfd, peer_addr );
        }
        else {
            ::close( connfd );
        }

    }
}



