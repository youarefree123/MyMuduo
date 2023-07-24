#include <string>
#include <stdio.h>
#include <unistd.h>

#include "base/thread_wrapper.h"
#include "base/current_thread.h"
#include "base/log.h"


void mysleep(int seconds)
{
    timespec t = { seconds, 0 };
    nanosleep(&t, NULL);
}

void threadFunc()
{
    printf("pid = %d ,tid=%d\n", ::getpid(), CurrentThread::Tid());
}

void threadFunc2(int x)
{
     printf("threadFunc2(int x) tid=%d, x=%d\n", CurrentThread::Tid(), x);
}

void threadFunc3()
{
    printf("threadFunc3() tid=%d\n", CurrentThread::Tid());
    mysleep(1);
}

class Foo
{
public:
    explicit Foo(double x)
        : x_(x)
    {
    }

    void memberFunc()
    {
        printf("tid=%d, Foo::x_=%f\n", CurrentThread::Tid(), x_);
    }

    void memberFunc2(const std::string& text)
    {
        printf("tid=%d, Foo::x_=%f, text=%s\n", CurrentThread::Tid(), x_, text.c_str());
    }

private:
        double x_;
};

int main()
{
    ONLY_TO_CONSOLE; LOGINIT(); LOG_LEVEL_TRACE;
    printf("pid=%d, tid=%d\n", ::getpid(), CurrentThread::Tid());

    ThreadWrapper t1(threadFunc);
    t1.Start();
    printf("t1.tid=%d\n", t1.tid());
    t1.Join();

    ThreadWrapper t2(std::bind(threadFunc2, 42),
                    "thread for free function with argument");
    t2.Start();
    printf("t2.tid=%d\n", t2.tid());
    t2.Join();

    Foo foo(87.53);
    ThreadWrapper t3(std::bind(&Foo::memberFunc, &foo),
                    "thread for member function without argument");
    t3.Start();
    t3.Join();

    ThreadWrapper t4(std::bind(&Foo::memberFunc2, std::ref(foo), std::string("Shuo Chen")));
    t4.Start();
    t4.Join();

    {
        ThreadWrapper t5(threadFunc3);
        t5.Start();
        // t5 may destruct eariler than thread creation.
    }
    mysleep(2);
    {
        ThreadWrapper t6(threadFunc3);
        t6.Start();
        mysleep(2);
        // t6 destruct later than thread creation.
    }
    sleep(2);
    printf("number of created threads %d\n", ThreadWrapper::num_created());
}