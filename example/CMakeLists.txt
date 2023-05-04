cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

# 指定可执行文件名及源码
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

add_executable(echo echo.cpp)


target_include_directories (
    echo PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/spdlog/include
#   ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 指定链接库
target_link_libraries(
    echo PUBLIC
    net 
  ) 







