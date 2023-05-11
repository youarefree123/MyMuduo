#pragma once 

#include <functional>
#include <memory>
#include <vector>

#include "net/base/noncopyable.h"

class EventLoop;
class EventLoopThread;



/**
 * 实际给用户的EventLoop接口，可配置ThreadPool的数量,baseEventLoop采用轮询的方式分发Channel
*/
class EventLoopThreadPool : noncopyable
{

public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool( EventLoop* base_loop, const std::string& name_arg ); 
    ~EventLoopThreadPool(); /* .h中仅声明，因为EventLoopThread在本文件内只做了前向声明  */

    void set_thread_num( int nums ) { num_threads_ = nums; }
    bool started() const { return started_; }
    const std::string& name() const { return name_; }

    void Start( const ThreadInitCallback& cb = ThreadInitCallback() );
    // 轮询,在Start()之后才会被调用
    EventLoop* GetNextLoop();

private:
    EventLoop* base_loop_; /* 主loop,对应的线程就是实例化该类的线程 */
    std::string name_; /*该eventloopthread的name*/
    bool started_; /*断言中使用，是否已经执行*/
    int num_threads_; /*设置的执行loop的数量*/
    size_t next_; /*轮询需要使用，如果只有一个loop，每次next都是该唯一的loop*/
    std::vector<std::unique_ptr<EventLoopThread>> threads_; /*管理EventLoopThread的vector*/
    std::vector<EventLoop*> loops_; /*为什么还需要这个？*/
};
