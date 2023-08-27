#pragma once

#include "base/log.h"
#include "base/noncopyable.h"


// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;
class InetAddress;

class SocketWrapper : noncopyable
{
public:
    SocketWrapper() = default;
    explicit SocketWrapper( int sockfd ) 
        : sockfd_( sockfd ){}
    ~SocketWrapper();
    
    int fd() const { return sockfd_; }
    
    // return true if success
    // bool GetTcpInfo( struct tcp_info* ) const; 
    // bool GetTcpInfoString( char* buf, int len ) const;

    // 执行失败，直接abort
    void BindAddress( const InetAddress& localaddr );
    void Listen(); 

    /// On success, returns a non-negative integer that is
    /// a descriptor for the accepted socket, which has been
    /// set to non-blocking and close-on-exec. *peeraddr is assigned.
    /// On error, -1 is returned, and *peeraddr is untouched.
    int Accept(InetAddress* peeraddr);

    void ShutdownWrite();

    void set_tcp_nodelay( bool on ); /*设置是否关闭Nagle算法 TCP_NODELAY */
    void set_reuse_addr( bool on ); /* 重用地址 SO_REUSEADDR */
    void set_reuse_port( bool on ); /* 重用端口 SO_REUSEPORT */
    void set_keep_alive( bool on ); /* 是否开启长连接 SO_KEEPALIVE */

private:
    int sockfd_;
};

// 只支持网络套接字，TCP, 非阻塞， fork后子进程关闭,最后一个参数写0 也可以
// SocketWrapper CreateNonblockSocket() {
//     int sockfd = ::socket( AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC , 0 );
//     if( sockfd < 0 ) {
//         CRITICAL( "CreateNonblockFd failed." );
//     }
//     return SocketWrapper( sockfd );
// }
