#include <string>
#include <atomic>
#include <cassert>
#include <memory>
#include <semaphore.h>

#include "current_thread.h"
#include "threadwrapper.h"



/*静态成员变量需要在类外单独进行初始化*/
std::atomic<uint32_t> ThreadWrapper::num_created_ = 0;


ThreadWrapper::ThreadWrapper( ThreadFunc func, const std::string& name ) 
  : started_( false ),
    joined_( false ),
    tid_( 0 ),
    name_( name ),
    func_( std::move( func ) ) 
{
    set_default_name();
}

// 线程已经开启，并且没有Join的话，默认会detach
ThreadWrapper::~ThreadWrapper() {
    if( started_ && !joined_ ) {
        p_thread_->detach();
    }
}

/**
 *  通过二值信号量来保证新的thread已经获取tid后，再继续流程
 *  （牢记该类是记录新线程的所有信息，所以必须要获取到tid才能继续执行）
*/
void ThreadWrapper::Start() {
    assert( !started_ );
    started_ = true;
    sem_t sem;
    ::sem_init( &sem, false, 0 );
    
    // 移动语义实现智能指针赋值 
    p_thread_ = std::move( std::make_unique<std::thread>( 
        [&](){
            tid_ = CurrentThread::Tid(); 
            ::sem_post(&sem);
            // 开启新线程，执行其对应线程函数
            func_();
        }   
     ) );

    ::sem_wait(&sem); /*必须等待互殴去上面新线程的tid*/
    assert( tid_ > 0 );
} 

void ThreadWrapper::Join() {
    assert( started_ ); 
    assert( !joined_ );
    joined_ = true;
    p_thread_->join();
}


void ThreadWrapper::set_default_name() {
    int num = ++num_created_; 
    if( name_.empty() ) {
        char buf[32] = {0};
        snprintf( buf, sizeof buf, "Thread %d", num );
        name_ = buf; // 存在隐式转换 char[] -> string
    }
}