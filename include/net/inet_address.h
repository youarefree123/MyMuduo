#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
using std::string;

class InetAddress {
public:
    explicit InetAddress( uint16_t port = 80, string ip = "127.0.0.1" );
    explicit InetAddress( const sockaddr_in& addr ) 
        : sockaddr_(addr){} 
    static const sockaddr* sockaddr_cast(const struct sockaddr_in* addr);

    string ToIp() const;
    uint16_t ToPort() const;
    string ToIpPort() const;

    /***************这里const去掉会有一个经典错误*****************/
    const sockaddr_in* get_sockaddr() const { return &sockaddr_ ; } /*{}前必须加const修饰 */
    void set_sockaddr( const sockaddr_in& addr ) { sockaddr_ = addr; }
private:
    sockaddr_in sockaddr_;
    
};