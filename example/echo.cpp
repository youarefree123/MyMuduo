#include "net/tcp_server.h"
#include "base/log.h"

#include <string>
#include <functional>

class EchoServer
{
public:
    EchoServer(EventLoop *loop,
            const InetAddress &addr, 
            const std::string &name)
        : server_(loop, addr, name)
        , loop_(loop)
    {
        // 注册回调函数
        server_.set_conn_callback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
        );

        // 设置收到消息的回调
        server_.set_msg_callback(
            std::bind(&EchoServer::onMessage, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );

         // test set_server 设置线程初始化操作回调
        server_.set_thread_init_callback( 
            []( EventLoop* loop ){
                INFO( "This Loop is {}, call set_thread_init_callback()", reinterpret_cast<size_t>(loop) );
            }
        );

        // 设置合适的loop线程数量 loopthread， 0 用于操作
        server_.set_thread_num(0);
    }
    void Start()
    {
        server_.Start();
    }
private:
    // 连接建立或者断开的回调
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->Connected())
        {
            INFO("Connection UP : {}", conn->peer_addr().ToIpPort().c_str());
        }
        else
        {
            INFO("Connection DOWN : {}", conn->peer_addr().ToIpPort().c_str());
        }
    }

    // 可读写事件回调
    void onMessage(const TcpConnectionPtr &conn,
                Buffer *buf,
                Timestamp time)
    {
        DEBUG( "current loop = {},  The msg size = {}, time = {}", reinterpret_cast<size_t>( conn->loop() ) , buf->ReadableBytes() , string(time) );
        // std::string msg = buf->RetrieveAllAsString();
        // INFO( "msg = {}", msg );
        conn->Send(buf);
        conn->Shutdown(); // 写端   EPOLLHUP =》 closeCallback_
        
    }

    

    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    ONLY_TO_CONSOLE; LOGINIT(); LOG_LEVEL_TRACE; // 只输出在控制台，TRACE级别
    EventLoop loop;
    InetAddress addr(8000);
    
    EchoServer server(&loop, addr, "EchoServer-01"); // Acceptor non-blocking listenfd  create bind 

   

    server.Start(); // listen  loopthread  listenfd => acceptChannel => mainLoop =>
    loop.Loop(); // 启动mainLoop的底层Poller

    return 0;
}