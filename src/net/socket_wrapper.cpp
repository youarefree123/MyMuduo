#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <string.h>
#include "log.h"
#include "net/socket_wrapper.h"
#include "net/inet_address.h"

SocketWrapper::~SocketWrapper() {
    ::close( sockfd_ );
}

void SocketWrapper::BindAddress( const InetAddress& addr ) {
    if( ::bind( sockfd_, (sockaddr*)addr.get_sockaddr() , sizeof addr ) != 0 ) {
        CRITICAL(" BindAddress failed , fd = {}", sockfd_);
    }
}

void SocketWrapper::Listen() {
    if( ::listen( sockfd_, SOMAXCONN ) != 0 ) {
         CRITICAL(" Listen failed , fd = {}", sockfd_);
    }
}

/**
 * 将一个fd从完成三次握手的全连接队列里取出
*/
int SocketWrapper::Accept(InetAddress* peeraddr) {
    sockaddr_in addr;
    bzero( &addr, sizeof addr );
    socklen_t addrlen = 0;
    // SOCK_CLOEXEC : fork的子进程中用exec系列系统调用加载新的可执行程序之前，关闭子进程中fork得到的fd。
    // SOCK_NONBLOCK : 设置非阻塞
    int connfd = ::accept4( sockfd_, const_cast<sockaddr*>( InetAddress::sockaddr_cast(&addr) ), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC );
    
    // 标识accept 成功，设置对应地址
    if( connfd >= 0 ) {
        peeraddr->set_sockaddr( addr );
    }

    return connfd; /* 可能返回 -1, 需要调用者去做判断 */

}
    // 关闭写端
    void SocketWrapper::ShutdownWrite() {
        if( ::shutdown( sockfd_, SHUT_WR ) < 0 ) {
            CRITICAL( "SocketWrapper::ShutdownWrite" );
        }
    }

    /*设置是否关闭Nagle算法 TCP_NODELAY , 协议级别*/
    void SocketWrapper::set_tcp_nodelay( bool on ) {
        int optval = on ? 1 : 0;
        ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                    &optval, static_cast<socklen_t>(sizeof optval));
        // FIXME CHECK
    }

    /* 重用地址 SO_REUSEADDR socket级别，下同*/
    void SocketWrapper::set_reuse_addr( bool on ) {
        int optval = on ? 1 : 0;
        ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                    &optval, static_cast<socklen_t>(sizeof optval));
        // FIXME CHECK
    }

    /* 重用端口 SO_REUSEPORT */
    void SocketWrapper::set_reuse_port( bool on ) {
    #ifdef SO_REUSEPORT
        int optval = on ? 1 : 0;
        int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                            &optval, static_cast<socklen_t>(sizeof optval));
        if (ret < 0 && on)
        {
            CRITICAL( "SO_REUSEPORT failed." );
        }
    #else
        if (on)
        {
            LOG_ERROR << "SO_REUSEPORT is not supported.";
        }
    #endif
    }

    /* 是否开启长连接 SO_KEEPALIVE */
    void SocketWrapper::set_keep_alive( bool on ) {
        int optval = on ? 1 : 0;
        ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                    &optval, static_cast<socklen_t>(sizeof optval));
        // FIXME CHECK
    }


