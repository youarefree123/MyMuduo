#include "net/poller.h"
#include "net/channel.h"


Poller::Poller( EventLoop* loop )
  :owner_loop_( loop ) {}

Poller::~Poller() = default;

bool Poller::HasChannel( Channel* channel ) const {
    auto it = channels_.find( channel->fd() );
    return it != channels_.end() && it->second == channel;
}

