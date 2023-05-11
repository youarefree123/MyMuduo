#include <cassert>

#include "net/tcp_server.h"
#include "net/event_loop.h"
#include "net/eventloop_thread_pool.h"
#include "log.h"
#include "net/acceptor.h"

static inline EventLoop* CHECK_NOTNULL( EventLoop* loop ) {
    if( loop == nullptr ) {
        CRITICAL("CHECK_NOTNULL failed.");
    }
    return loop;
}

// conn_cb_ 等默认回调目前没写，后面需要add
TcpServer::TcpServer( EventLoop* loop, 
                      const InetAddress& listen_addr, 
                      const std::string& name_arg,
                      Option opt )
 :  loop_( CHECK_NOTNULL( loop ) ),
    ip_port_( listen_addr.ToIpPort() ),
    name_( name_arg ),
    p_acceptor_( new Acceptor( loop, listen_addr, opt == kReusePort ) ),
    p_thread_pool_( new EventLoopThreadPool( loop, name_ ) ),
    conn_cb_(  ), 
    msg_cb_( ),
    started_(0),
    next_connid_( 1 )
    
{
    // 当有新用户连接时，需要执行TcpServer 的 NewConnection 回调
    // 有两个占位符，填的分别是sockfd和对应addr
    p_acceptor_->set_new_conn_cb( 
        std::bind( &TcpServer::NewConnection,
                   this,
                   std::placeholders::_1,
                   std::placeholders::_2 )
    );

}


TcpServer::~TcpServer() {
    loop_->AssertInLoopThread();
    TRACE( "~TcpServer" );

    /* 销毁每个连接 */
    for( auto& it : conns_ ) {
        TcpConnectionPtr p_conn(it.second); 
        // it.second.reset() 后， tcpserver 本身的那个强智能指针就无法访问对应资源了
        //  而局部智能智能对象 p_conn 作用域外又能直接析构掉
        // 如果直接p_conn.reset(), 就无法调用下面的那个回调了
        it.second.reset(); 
        p_conn->loop()->RunInLoop(
            std::bind( &TcpConnection::ConnectDestroyed, p_conn )
        ); 
    }

}

void TcpServer::set_thread_num( int num_threads ) {
    assert( num_threads >= 0 );
    p_thread_pool_->set_thread_num( num_threads );
} 
  
// 初始化所有SubLoop， acceptor设置监听
void TcpServer::Start() {
    // started_ 原子的，保证start实际只执行一次
    if( started_++ == 0 ) {
        p_thread_pool_->Start( thread_init_cb_ );
        assert( !p_acceptor_->listening() );
        // unique_ptr 没有复制语义，所以不能直接传
        // 那为啥之前不用shared_ptr? 是因为根据事实的逻辑，acceptor只能mainloop用吗？
        loop_->RunInLoop(
            std::bind( &Acceptor::Listen, p_acceptor_.get() )
        );
    }
    
}


/// Not thread safe, but in loop
/**
 * 每当有一个新的Conn连接，Acceptor就会执行一次该回调，来绑定这个对端连接的fd和地址，并分发
 * 
*/
void TcpServer::NewConnection(int sockfd, const InetAddress& peer_addr) {
    EventLoop* io_loop = p_thread_pool_->GetNextLoop(); /* 轮询算法得到一个IOLoop ,如果没有设置多loop的情况下每次返回的都是mainloop*/
    char buf[64] = {0};
    snprintf( buf,sizeof buf, "-%s#%d", ip_port_.c_str(), next_connid_ ); /* 设置新conn的名程 */
    ++next_connid_; /* NewConnection只会在main thread中执行，所以不需要考虑多线程 */
    std::string conn_name = name_ + buf;

    INFO( "TcpServer::newConnection [{}] - new connection [{}] from {}.",
         name_, conn_name, peer_addr.ToIpPort() );
    
    // 通过sockfd 获取其绑定的主机ip 和端口, 即获取本机InetAddress
    sockaddr_in local;
    ::bzero(&local, sizeof local);
    socklen_t addrlen = sizeof local;

    // 从已连接的fd的四元组（本地ip port， 远端ip port）中拿到本机地址
    if (::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0)
    {
        ERROR("sockets::getLocalAddr");
    }
    InetAddress local_addr(local);

    // 根据连接成功的sockfd，创建TcpConnection连接对象
    TcpConnectionPtr conn(
        new TcpConnection( io_loop, conn_name, sockfd, local_addr, peer_addr )
    );
    conns_[conn_name] = conn;

    // 下面的回调都是用户设置给TcpServer=>TcpConnection=>Channel=>Poller=>notify channel调用回调
    conn->set_conn_cb(conn_cb_);
    conn->set_msg_cb(msg_cb_);
    conn->set_written_cb(write_complete_cb_);

    // 设置了如何关闭连接的回调   conn->shutdown()，注册关闭conn的回调
    conn->set_close_cb(
        std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1)
    );

     // 直接调用TcpConnection::connectEstablished, 设置连接完成
    io_loop->RunInLoop(
        std::bind( &TcpConnection::ConnectEstablished, conn )
    );

}


/// Thread safe.
void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
    loop_->RunInLoop(
        std::bind( &TcpServer::RemoveConnectionInLoop, this, conn )
    );
}

/// Not thread safe, but in loop
void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr& conn) {
    INFO( "TcpServer::removeConnectionInLoop [%s] - connection %s", name_, conn->name() );
    conns_.erase( conn->name() );
    EventLoop* io_loop = conn->loop();
    io_loop->QueueInLoop(
        std::bind( &TcpConnection::ConnectDestroyed, conn )
    );
}
