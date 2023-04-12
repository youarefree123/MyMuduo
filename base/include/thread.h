// #include <thread>
// #include <functional>
// #include <memory>
// #include <string>
// #include <atomic>
// #include <sys/types.h>




// #include "noncopyable.h"

// using std::string;

// class Thread : noncopyable
// {
// public:
//     using ThreadFunc = std::function<void()> ;
//     explicit Thread( thread_func_, const string& name = string{} );
//     // todo 线程可以做移动构造和赋值
//     ~Thread();

//     void Start(); 
//     void Join();

//     bool started() const { return started_; }
//     pid_t tid() const { return tid_; } 
//     const string& name() const { return name_; }
//     static uint32_t num_created() const { return num_created_.get(); } 


// private:
//     void set_default_name();

//     bool started_; /*线程是否开启*/
//     bool joined_; /*线程是否是join*/
//     pid_t tid_; /*线程tid, 不等于pthread.self 的 那个id*/
//     std::unique_ptr< std::thread > p_thread_; /*线程(可不可以用unique_ptr)*/
//     string name_; /*线程名*/
//     ThreadFunc func_; /*线程函数*/
//     static std::atomic<uint32_t> num_created_; /*静态变量，记录已创建的线程数*/

// };

