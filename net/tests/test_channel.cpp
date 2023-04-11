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
    howlong.it_value.tv_sec = 5;
    howlong.it_interval.tv_sec = 1;
    // howlong.it_interval = {1, 0}; // period timeout value = 1s
    // howlong.it_value = {5, 100};  // initial timeout value = 5s100ns

    ::timerfd_settime( timerfd, TFD_TIMER_ABSTIME, &howlong, NULL );

    // for (int i = 0; i < 5 ; ++i) {
    //     uint64_t exp = 0;
    //     uint64_t s = 0;    
    //     s = read(timerfd, &exp, sizeof(uint64_t));
    //     INFO( "read:{}, i = {}", exp, i );
        
    // }


    loop.Loop();
    channel.DisableAll();
    channel.Remove();

    ::close( timerfd );
  
  return 0;
}


// #include <iostream>
// #include <sys/timerfd.h>
// #include <poll.h>
// #include <unistd.h>
// #include <assert.h>

// using namespace std;

// int main() {
//     struct itimerspec timebuf;
//     int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
//     timebuf.it_interval = {1, 0}; // period timeout value = 1s
//     timebuf.it_value = {5, 100};  // initial timeout value = 5s100ns
//     timerfd_settime(timerfd, 0, &timebuf, NULL);

//     struct pollfd fds[1];
//     int len = sizeof(fds) / sizeof(fds[0]);
//     fds[0].fd = timerfd;
//     fds[0].events = POLLIN | POLLERR | POLLHUP;

//     while (true)
//     {
//         int n = poll(fds, len, -1);
//         for (int i = 0; i < len && n-- > 0; ++i) {
//             if (fds[i].revents & POLLIN)
//             {
//                 uint64_t val;
//                 int ret = read(timerfd, &val, sizeof(val));
//                 if (ret != sizeof(val)) // ret should be 8
//                 {
//                     cerr << "read " << ret << "bytes instead of 8 frome timerfd" << endl;
//                     break;
//                 }
//                 cout << "timerfd = " << timerfd << " timeout!" << endl;
//             }
//         }
//     }
//     close(timerfd);
//     return 0;
// }
