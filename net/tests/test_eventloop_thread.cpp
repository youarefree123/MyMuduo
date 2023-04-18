#include "eventloop_thread_pool.h"
#include "event_loop.h"
#include "thread_wrapper.h"
#include "log.h"

#include <stdio.h>
#include <unistd.h>


void print(EventLoop* p = NULL)
{
  printf("main(): pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::Tid(), p);
}

void init(EventLoop* p)
{
  printf("init(): pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::Tid(), p);
}

int main()
{
  print();

  EventLoop loop;
//   loop.runAfter(11, std::bind(&EventLoop::Quit, &loop));

//   {
//     printf("Single thread %p:\n", &loop);
//     EventLoopThreadPool model(&loop, "single");
//     model.set_thread_num(0);
//     model.Start(init);
//     assert(model.GetNextLoop() == &loop);
//     assert(model.GetNextLoop() == &loop);
//     assert(model.GetNextLoop() == &loop);
//   }

  {
    printf("Another thread:\n");
    EventLoopThreadPool model(&loop, "another");
    model.set_thread_num(1);
    model.Start(init);
    EventLoop* nextLoop = model.GetNextLoop();
    // nextLoop->runAfter(2, std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop == model.GetNextLoop());
    assert(nextLoop == model.GetNextLoop());
    ::sleep(3);
  }

  {
    printf("Three threads:\n");
    EventLoopThreadPool model(&loop, "three");
    model.set_thread_num(3);
    model.Start(init);
    EventLoop* nextLoop = model.GetNextLoop();
    nextLoop->RunInLoop(std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop != model.GetNextLoop());
    assert(nextLoop != model.GetNextLoop());
    assert(nextLoop == model.GetNextLoop());
  }

  loop.Loop();
}



// void print(EventLoop* p = NULL)
// {
//   printf("print: pid = %d, tid = %d, loop = %p\n",
//          getpid(), CurrentThread::Tid(), p);
// }

// void quit(EventLoop* p)
// {
//   print(p);
//   p->Quit();
// }

// int main()
// {
//     ONLY_TO_CONSOLE; LOGINIT(); LOG_LEVEL_DEBUG;
//     print();

//   {
//   EventLoopThread thr1;  // never Start
//   }

//   {
//   // dtor calls quit()
//   EventLoopThread thr2;
//   EventLoop* loop = thr2.StartLoop();
//   loop->RunInLoop(std::bind(print, loop));
//   sleep(1);
// //   CurrentThread::SleepUsec(500 * 1000);
//   }

//   {
//   // quit() before dtor
//   EventLoopThread thr3;
//   EventLoop* loop = thr3.StartLoop();
// //   loop->RunInLoop(std::bind(quit, loop));
//     loop->RunInLoop(std::bind(print, loop));
// //   CurrentThread::SleepUsec(500 * 1000);
//   }
// }