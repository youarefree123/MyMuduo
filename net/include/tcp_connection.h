#pragma once 

#include "event_loop.h"

class TcpConnection
{

public:
    TcpConnection(/* args */);
    ~TcpConnection();
    void ConnectDestroyed() {}
    EventLoop* loop() const { return loop_; }
private:
    EventLoop* loop_;
};

TcpConnection::TcpConnection(/* args */)
{
}

TcpConnection::~TcpConnection()
{
}
