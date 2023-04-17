#include "eventloop_thread.h"
#include "event_loop.h"
#include "log.h"
#include <stdio.h>
#include <unistd.h>



void print(EventLoop* p = NULL)
{
  printf("print: pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::Tid(), p);
}

void quit(EventLoop* p)
{
  print(p);
  p->Quit();
}

int main()
{
    ONLY_TO_CONSOLE; LOGINIT(); LOG_LEVEL_DEBUG;
    print();

  {
  EventLoopThread thr1;  // never start
  }

  {
  // dtor calls quit()
  EventLoopThread thr2;
  EventLoop* loop = thr2.StartLoop();
  loop->RunInLoop(std::bind(print, loop));
  sleep(1);
//   CurrentThread::SleepUsec(500 * 1000);
  }

  {
  // quit() before dtor
  EventLoopThread thr3;
  EventLoop* loop = thr3.StartLoop();
//   loop->RunInLoop(std::bind(quit, loop));
    loop->RunInLoop(std::bind(print, loop));
//   CurrentThread::SleepUsec(500 * 1000);
  }
}