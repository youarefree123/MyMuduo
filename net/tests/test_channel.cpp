#include "log.h"
#include "timestamp.h"
#include "channel.h"
#include "event_loop.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>

using namespace std;

/**
 * 利用timer fd 实现一个单次触发的定时器
 * 将timer fd 的可读事件转发给timerout（）
*/

EventLoop* g_loop;

void timeout( Timestamp now_time ) {
    printf( "Timeout!\n" );
    g_loop->Quit();
}

int main(int argc, char const *argv[])
{
    ONLY_TO_CONSOLE; LOGINIT(); LOG_LEVEL_TRACE;
    EventLoop loop{};
    g_loop = &loop;
    int timerfd = ::timerfd_create( CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC  );
    Channel channel( &loop, timerfd );

    // 布置现场
    channel.set_read_callback( timeout );
    channel.EnableReading();

    struct itimerspec howlong;
    bzero( &howlong, sizeof howlong );
    howlong.it_interval.tv_sec = 5;
    ::timerfd_settime( timerfd, 0, &howlong, NULL );

    loop.Loop();

    ::close( timerfd );
  
  return 0;
}
