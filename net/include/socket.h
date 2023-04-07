#pragma once

class Socket
{
public:
    Socket() = default;
    explicit Socket( int sockfd ) 
        : sockfd_( sockfd ){}
    ~Socket();
    
    int fd() const { return sockfd_; }
    

private:
    int sockfd_;
};
