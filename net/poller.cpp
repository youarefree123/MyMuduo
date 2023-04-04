#include "poller.h"
#include "channel.h"


Poller::Poller( EventLoop* loop )
  :owner_loop_( loop ) {}

Poller::~Poller() = default;

bool Poller::HasChannel( Channel* channel ) const {
  auto it = channels_.find( channel->fd() );
  return it != channels_.end() && it->second == channel;
}

const char* OperationToString( int op ) const {
  switch (op)
  {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}