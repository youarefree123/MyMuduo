#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
using std::string;

class InetAddress {
public:
    explicit InetAddress( uint16_t port = 80, string ip = "127.0.0.1" );
    explicit InetAddress( const InetAddress& addr ) 
        : sockaddr_(addr.sockaddr()){} 
    
    string ToIp() const;
    uint16_t ToPort() const;
    string ToIpPort() const;

    const sockaddr_in& sockaddr() const { return sockaddr_; } 

private:
    sockaddr_in sockaddr_;
};