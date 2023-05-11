#pragma once

#include <mutex>
#include <string>
#include <condition_variable>

#include "net/base/noncopyable.h"
#include "net/base/thread_wrapper.h"

/**
 *  绑定一个thread 和 loop ，打包成一个类
*/
class EventLoop;

class EventLoopThread : noncopyable
{

public:
    // 用于eventloop初始化
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    explicit EventLoopThread( const ThreadInitCallback& cb = ThreadInitCallback(nullptr),
                             const std::string& name = std::string() );
    ~EventLoopThread();
    EventLoop* StartLoop();
private:
    void thread_func(); /*每个线程需要执行的函数*/
    EventLoop* loop_; /*需要锁*/
    bool exiting_; /*是否退出了*/
    ThreadWrapper thread_; /*每个loop对应绑定的thread*/
    std::mutex mtx_; /*c++11 互斥锁*/
    std::condition_variable cond_; /*c++11 条件变量*/
    ThreadInitCallback cb_; /*用于对eventloop做初始化*/
};

