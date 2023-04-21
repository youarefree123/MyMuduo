

#include "log.h"
#include "tcp_connection.h"
#include "callbacks.h"

TcpConnection::TcpConnection( EventLoop* loop,
                            const std::string name,
                            const InetAddress& local_addr,
                            const InetAddress& peer_addr ){}
TcpConnection::~TcpConnection(){}


// void send(string&& message); // C++11
// void send(Buffer&& message); // C++11
void TcpConnection::Send( const void* message, int len ){} /* C++11是不是可以优化？ */
void TcpConnection::Shutdown(){} /* 线程不安全 */
void TcpConnection::ConnectEstablished(){} /* 当新连接被accept 调用,只调一次 */

void TcpConnection::ConnectDestroyed(){} /* 当新连接被移除调用,只调一次 */
   