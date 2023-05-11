#pragma once 

#include <vector>
#include "net/poller.h"

struct epoll_event;

class EpollPoller : public Poller
{
public:
    using ChannelList = std::vector<Channel*>;
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override ;

    Timestamp Poll( int timeout_ms, ChannelList* active_channels ) override;
    void UpdateChannel( Channel* channel ) override;
    void RemoveChannel( Channel* channel ) override;
private:
    static const int kInitEventListSize = 16; // 默认初始wait最多一次性接收16个事件
    
    // 填满ChannelList ？
    void FillActiveChannels( int num_events, ChannelList* active_channels ) const;
    // 更新channel的事件
    void Update( int operation, Channel* channel );
    const char* OperationToString( int op ) const;


    int epollfd_;
    using EventList = std::vector<struct epoll_event>;
    EventList events_list_;   // 保存所监听的fd返回的事件
};


