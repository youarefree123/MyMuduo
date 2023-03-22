#pragma once

// 继承自NonCopyable的类可以正常构造和析构，但是不能进程拷贝构造和拷贝赋值
class NonCopyable {
public:
  NonCopyable( const NonCopyable& ) = delete;
  void operator=( const NonCopyable&) = delete;

protected:
  NonCopyable() = default;
  ~NonCopyable() = default;

};