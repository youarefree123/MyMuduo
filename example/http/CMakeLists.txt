
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

# 指定可执行文件名及源码
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin")

# 只遍历当前目录
file(GLOB  SRC_DIR_LIST "./*.cpp")
message("http---${PROJECT_SOURCE_DIR}")

add_executable( http ${SRC_DIR_LIST})

# 指定链接库
target_link_libraries(
    http PUBLIC
    net 
  ) 












