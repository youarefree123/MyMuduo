#pragma once 

#include <vector>
#include <sys/epoll.h>

class Poller;
class EventLoop;

class EpollPoller
{
public:
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


  int epollfd_;
  using EventList = std::vector<struct epoll_event>;
  EventList events_;
};


