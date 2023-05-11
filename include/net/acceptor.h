#pragma once 

#include <functional>

#include "net/inet_address.h"
#include "net/base/noncopyable.h"
#include "net/socket_wrapper.h"
#include "net/channel.h"

class EventLoop;
class InetAddress;

/**
 * 仅与主loop配合调用，监听并接收到来的TCP连接
*/

class Acceptor 
{
public:
    using NewConnCallback = std::function<void(int sockfd, const InetAddress&)>;

    explicit Acceptor( EventLoop* loop, const InetAddress& listen_addr, bool reuse_port );
    ~Acceptor();

    void set_new_conn_cb( const NewConnCallback& cb ) {
        new_conn_cb_ = cb;
    }

    void Listen(); 
    bool listening() const { return listening_; }
    
private:
    void HandleRead();
    EventLoop* loop_; /* 默认绑定主loop */
    SocketWrapper listen_socket_; 
    Channel listen_channel_; /* 绑定上面的socket */
    NewConnCallback new_conn_cb_; /* */
    bool listening_; /* 用于是否正在监听 */
    int idle_fd_; /* 预留的一个fd位置，默认开启，如果fd达到上限，就关闭他来accet那个连接，然后直接关闭 */
};

