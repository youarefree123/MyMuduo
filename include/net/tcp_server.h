#pragma once 


#include <string>
#include <atomic>


#include "net/event_loop.h"
#include "net/acceptor.h"
#include "net/tcp_connection.h"
#include "net/inet_address.h"
#include "net/base/noncopyable.h"
#include "net/eventloop_thread_pool.h"

/*  给用户使用的类，所以不用前置声明，直接导入头文件，后续用户使用的时候就不需要再导相应的包了 
    核心类，管理Acceptor和所有conn
*/
class TcpServer
{
public:
    // EvenetLoopThread 的 thread_func() 调用
    using ThreadInitCallback = std::function<void( EventLoop* )>;
   

    // 是否重用端口
    enum Option {
        kNousePort = 0,
        kReusePort,
    };

    TcpServer(  EventLoop* loop, 
                const InetAddress& listen_addr, 
                const std::string& name_arg,
                Option opt = kNousePort );
    ~TcpServer(); // force out-line dtor, for std::unique_ptr members.

    const std::string& ip_port() const { return ip_port_; }
    const std::string& name() const { return name_; }
    EventLoop* loop() const { return loop_; }

    void set_thread_num( int num_threads ); 

    // 默认为空
    void set_thread_init_callback( const ThreadInitCallback& cb ) { 
        thread_init_cb_ = cb;
    }
    std::shared_ptr<EventLoopThreadPool> thread_pool() {
        return p_thread_pool_;
    }

    void Start(); /* 线程安全 */

    // 下面的设置回调都是线程不安全的 
    void set_conn_callback( const ConnectionCallback& cb ) {
        conn_cb_ = cb;
    }
    void set_msg_callback( const MessageCallback& cb ) {
        msg_cb_ = cb;
    }

    // 写完成，低水位回调
    void set_written_callback( const WriteCompletedCallback& cb ) {
        write_complete_cb_ = cb;
    }


private:

    using ConnctionMap = std::unordered_map< std::string, TcpConnectionPtr >;

    /// Not thread safe, but in loop
    void NewConnection(int sockfd, const InetAddress& peerAddr);
    /// Thread safe.
    void RemoveConnection(const TcpConnectionPtr& conn);
    /// Not thread safe, but in loop
    void RemoveConnectionInLoop(const TcpConnectionPtr& conn);


    EventLoop* loop_; /* baseloop, 用户自定义 */
    const std::string ip_port_; /* 格式举例: 127.0.0.0:80 */
    const std::string name_; 
    std::unique_ptr<Acceptor> p_acceptor_; /* 只有主loop中才会有Acceptor */
    std::shared_ptr<EventLoopThreadPool> p_thread_pool_; /* one loop per thread的核心，分发任务给其他loop */

    ConnectionCallback conn_cb_; /* 有新连接时的回调 */
    MessageCallback msg_cb_; /* 有读写消息时的回调 */
    WriteCompletedCallback write_complete_cb_; /* 消息发送完成的回调 */

    ThreadInitCallback thread_init_cb_; /* 线程初始化回调 */
    std::atomic<int> started_; /*需要初始化，保证一个server只能被start一次*/
    
    int next_connid_; /*  */ 
    ConnctionMap conns_; /* 维护所有连接 k:v */
};