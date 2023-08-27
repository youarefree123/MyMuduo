#pragma once 

#include <memory>
#include <string>
#include <atomic>

#include "net/callbacks.h"
#include "base/buffer.h"
// #include "base/unlimited_buffer.h"
#include "net/inet_address.h"
#include "base/noncopyable.h"
#include "base/timestamp.h"
class Channel;
class EventLoop;
class SocketWrapper;





/**
 *  TcpServer => Acceptor -> 有新客户端连接，通过accept 拿到connfd和addr
 * 
 *  => 打包成TcpConnection,设置回调 => 对应Channel回调 => 注册到Poller上
 *  
 *  => 事件发生, 执行相应回调
 * 
 *  Acceptor 和 TcpConnection的区别: Acceptor负责处理新连接，TcpConnection负责处理后续
 * 
 *
 *  关于为什么要继承enable_shared_from_this，且绑定回调的时候为什么不用this
 *  
 *  1.把当前类对象作为参数传给其他函数时，为什么要传递share_ptr呢？直接传递this指针不可以吗？

　　一个裸指针传递给调用者，谁也不知道调用者会干什么？假如调用者delete了该对象，而share_tr此时还指向该对象。

　　2.这样传递share_ptr可以吗？share_ptr <this>

　　这样会造成2个非共享的share_ptr指向一个对象，最后造成2次析构该对象。
 * 
 *  
*/

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{

public:

    // using Buffer = UnlimitedBuffer; 
    // using Buffer = BaseBuffer; /* 使用原buffer */ 

    explicit TcpConnection( EventLoop* loop,
                            const std::string name,
                            int sockfd,
                            const InetAddress& local_addr,
                            const InetAddress& peer_addr );
    ~TcpConnection();

    EventLoop* loop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& local_addr() const { return local_addr_; }
    const InetAddress& peer_addr() const { return peer_addr_; }
    bool Connected() const { return state_ == kConnected; }
    bool Disconnected() const { return state_ == kDisconnected; }
    void SetTcpNoDelay( bool flag ) ;

    // void Send( const string&  buf); // C++11
    void Send( Buffer* buf );
    // void Send(UnlimitedBuffer&& message); // C++11
    // void Send( const void* message, int len ); /* C++11是不是可以优化？ */


    void Shutdown(); /* 线程不安全 */
    void ShutdownInLoop(); 

    // 设置回调
    void set_conn_cb( const ConnectionCallback& cb ) {
        conn_cb_ = cb;
    }

    void set_msg_cb( const MessageCallback& cb ) {
        msg_cb_ = cb;
    }

    void set_written_cb( const WriteCompletedCallback& cb ) {
        write_complete_cb_ = cb;
    }

    void set_high_water_mark_cb( const HighWaterMarkCallback& cb , size_t high_water_mark ) {
        high_water_mark_cb_ = cb;
        high_water_mark_ = high_water_mark;
    }

    // 类内部使用
    void set_close_cb( const CloseCallback& cb ) { 
        close_cb_ = cb; 
    }

    /// Advanced interface
    // UnlimitedBuffer* input_buf()
    // { return &input_buffer_; }

    // Buffer* output_buffer()
    // { return &output_buffer_; }
    
    void ConnectEstablished(); /* 当新连接被accept 调用,只调一次 */

    void ConnectDestroyed(); /* 当新连接被移除调用,只调一次 */

    
   

private:
    enum StateE {
        kDisconnected = 0, /* 已断开 */
        kConnecting, /* 正在连接 */
        kConnected, /* 已连接 */
        kDisconnecting /* 正在断开 */
    };

    void set_state( StateE s ) { state_ = s; }

    // 每个连接对应的回调处理函数
    void HandleRead( Timestamp receive_time );
    void HandleWrite();
    void HandleClose();
    void HandleError();
    
    void SendInLoop(const void* message, size_t len);
    void SendInLoop( const string& message );
    
    EventLoop* loop_; 
    const std::string name_;
    std::atomic<int> state_; 
    bool reading_; 

    std::unique_ptr<SocketWrapper> socket_; /* 管理conn对应的那个socket类 */
    std::unique_ptr<Channel> channel_; /* 以及管理其对应的channel（fd对应唯一的socket和channel？） */
    
    const InetAddress local_addr_; /* 当前主机的地址 */
    const InetAddress peer_addr_; /* 客户端的地址 */

    // 用户通过TcpServer接口设置的回调，实际上会扔给TcpConn类（然后TcpConn扔给Channel）
    ConnectionCallback conn_cb_; /* 有新连接时的回调 */
    MessageCallback msg_cb_; /* 有读写消息时的回调 */
    WriteCompletedCallback write_complete_cb_; /* 消息发送完成的回调 */
    CloseCallback close_cb_; /* 关闭连接对应的回调 */
    HighWaterMarkCallback high_water_mark_cb_; /* 超过高水位的回调 */

    size_t high_water_mark_; /* 高水位线 */

    // UnlimitedBuffer input_buffer_;  /* 接收缓冲区（conn接收） */
    // UnlimitedBuffer output_buffer_; /* 发送缓冲区( conn发送 ) */

    Buffer input_buffer_;  /* 接收缓冲区（conn接收） */
    Buffer output_buffer_; /* 发送缓冲区( conn发送 ) */

};