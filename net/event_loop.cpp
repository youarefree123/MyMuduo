#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "log.h"
#include "event_loop.h"

namespace {

__thread EventLoop* t_loop_in_this_thread = nullptr;

const int KPollTimeMs = 10000;

// 创建wakeup_fd
int CreatEventFd() {
    // CLOEXEC 在fork的子进程中用exec系列系统调用加载新的可执行程序之前，关闭子进程中fork得到的fd。
    int evtfd = ::eventfd( 0, EFD_NONBLOCK | EFD_CLOEXEC );
    if( evtfd < 0 ) {
        CRITICAL( "Failed in eventfd" );
    }
    return evtfd;
}

// (忽略信号的？？)不知道干啥的，先拷贝过来
#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;

}  // namespace



/*
    EventLoop 类函数
*/

EventLoop* EventLoop::GetEventLoopOfCurrentThread() {
    return t_loop_in_this_thread;
}

EventLoop::EventLoop() 
  : looping_( false ),
    quit_( false ),
    event_handling_( false ),
    calling_pending_functors_( false ),
    thread_id( CurrentThread::Tid() ),
    p_poller_( Poller::NewPoller( this ) ),
    wakeup_fd_( CreatEventFd() ),
    p_wakeup_channel_( new Channel( this, wakeup_fd_ ) ),
    current_active_channel_( nullptr )
{
    DEBUG( "EventLoop created,tid:{}",thread_id );
    if( t_loop_in_this_thread ) {
        CRITICAL( "Another EventLoop {} exists in this thread", t_loop_in_this_thread );
        
    }


}

