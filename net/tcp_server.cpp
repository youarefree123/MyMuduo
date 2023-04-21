#include <cassert>

#include "tcp_server.h"
#include "event_loop.h"
#include "eventloop_thread_pool.h"
#include "log.h"
#include "acceptor.h"

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
    for( auto& [k,v_conn] : conns_ ) {
        v_conn.reset(); // v引用计数-1, v 是 shared_ptr
        v_conn->loop()->RunInLoop(
            std::bind( &TcpConnection::ConnectDestroyed, v_conn )
        ); 
    }

}

void TcpServer::set_thread_num( int num_threads ) {
    assert( num_threads >= 0 );
    p_thread_pool_->set_thread_num( num_threads );
} 
  
//线程开启，然后主loop执行监听
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
void TcpServer::NewConnection(int sockfd, const InetAddress& peerAddr){}
/// Thread safe.
void TcpServer::RemoveConnection(const TcpConnectionPtr& conn){}
/// Not thread safe, but in loop
void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr& conn){}
