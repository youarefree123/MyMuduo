
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include "net/inet_address.h"

using std::string;

InetAddress::InetAddress( uint16_t port, string ip ) {
    bzero( &sockaddr_, sizeof(sockaddr_) );  // 清空
    sockaddr_.sin_family = AF_INET; // 协议族 TCP
    sockaddr_.sin_port = htons(port); // 端口转网络字节序
    sockaddr_.sin_addr.s_addr = inet_addr(ip.c_str()); // 点分ip转网络地址
}

string InetAddress::ToIp() const {
     return inet_ntoa( sockaddr_.sin_addr ); // 转回本地字节序
}

uint16_t InetAddress::ToPort() const {
     return ntohs( sockaddr_.sin_port ); // 端口也是
}

string InetAddress::ToIpPort() const {
      return ToIp() + " :" + std::to_string( ToPort() );
}

const struct sockaddr* InetAddress::sockaddr_cast(const struct sockaddr_in* addr) {
    return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
}