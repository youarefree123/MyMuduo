#pragma once

#include <functional>
#include <memory>
#include <string>
using std::string;

#include "net/base/noncopyable.h"
#include "net/base/timestamp.h"

class EventLoop;


// 封装了sockfd 及其 感兴趣的事件，每个Channel都绑定了唯一的loop

class Channel : noncopyable
{
public:
    explicit Channel( EventLoop* loop, size_t fd );
    ~Channel();


    // 事件回调
    using EventCallback = std::function< void() >; 
    using ReadEventCallback = std::function< void(Timestamp) >;
    // fd 得到poller通知以后，处理事件
    void HandleEvent( Timestamp receive_time );

    // 设置回调函数对象
    void set_read_callback( ReadEventCallback cb ) {
        read_callback_ = std::move( cb );
    }
    void set_write_callback( EventCallback cb ) {
        write_callback_ = std::move( cb );
    }
    void set_close_callback( EventCallback cb ) {
        close_callback_ = std::move( cb );
    }
    void set_error_callback( EventCallback cb ) {
        error_callback_ = std::move( cb );
    }

  // 防止当channel被手动remove后，channel 还在执行回调
  // 什么时候调用过？
    void Tie( const std::shared_ptr< void >& );

    int fd() const { return fd_; }

    int events() const { return events_; }

    void set_revents( int revt ) {
        revents_ = revt;
    }
 
    // 设置fd读事件
    void EnableReading() { events_ |= kReadEvent; Update(); }
    void DisableReading () { events_ &= ~kReadEvent; Update(); }

    void EnableWriting() { events_ |= kWriteEvent; Update(); }
    void DisableWriting () { events_ &= ~kWriteEvent; Update(); }
    void DisableAll() {events_ = kNoneEvent; Update(); }

    // 返回fd当前注册的事件状态
    bool IsNoneEvent() const { return events_ == kNoneEvent; }
    bool IsReading() const { return events_ & kReadEvent; }
    bool IsWriting() const { return events_ & kWriteEvent; }

    int fd_status() { return fd_status_; }
    void set_fd_status( int idx ) { fd_status_ = idx; }

    // 返回Channel所属的loop
    EventLoop* owner_loop() const { return loop_; }  

    // 更新感兴趣事件
    void Update(); 
    void Remove();
    // recetive_time是poll返回后那个时间戳
    void HandleEventWithGuard( Timestamp recetive_time ); 

    string EventsToString(int ev);



private:
    static const int kNoneEvent;
    static const int kReadEvent ; // 读事件 , 当有带外数据时候触发 EPOLLPRI 
    static const int kWriteEvent;

    const int fd_;    // 绑定的fd
    EventLoop* loop_; // 绑定的loop
    int events_;      // 已注册的fd感兴趣事件
    int revents_;     // poller返回的已触发的事件
    int fd_status_ ;  // 保存fd当前状态：已监听、未监听、已移除

    // 防止Channel被手动remove后还在使用，tie进行跨线程的生存状态监听
    std::weak_ptr<void> tie_; 
    bool tied_; 

    // 只有Channel才知道具体回调
    ReadEventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;

};


