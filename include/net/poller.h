#pragma once

#include <vector>
#include <unordered_map>

#include "net/base/timestamp.h"

class Channel;
class EventLoop;


class Poller 
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller (EventLoop* loop);
    virtual ~Poller ();

    // epoll_wait 接口，返回实际等待时间？？
    // active_channels 表示活跃连接
    virtual Timestamp Poll( int timeout_ms, ChannelList* active_channels ) = 0;

    // 更新Channel （更新事件等？？）
    virtual void UpdateChannel( Channel* channel ) = 0;
    // 从loop中移除Channel
    virtual void RemoveChannel( Channel* channel ) = 0;
    // 判断poller 中是否拥有某个Channel
    virtual bool HasChannel( Channel* channel ) const;
    // 返回默认的Poller 对象
    static Poller* NewPoller( EventLoop* loop );


  
protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_; // fd 到 Channel 的映射

private:
    EventLoop* owner_loop_;
};

