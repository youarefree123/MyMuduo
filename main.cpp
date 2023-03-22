#include "buffer.h"
#include "noncopyable.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
// #include <deque>
// #include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
  // deque<int> a{1};
  // cout<<a.front()<<endl;
  Buffer b{};
  // 开启并创建本地日志
  auto  my_logger = spdlog::basic_logger_mt("file_logger", "../logs/basic-log");

  // 设置该日志的显示级别
  my_logger->set_level(spdlog::level::info);

  // 向该日志中写入信息
  my_logger->info("Hello, {}!", "World");
  return 0;
}
