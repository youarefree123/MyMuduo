cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../lib")
# 遍历当前目录和子目录
# file(GLOB_RECURSE SRC_DIR_LIST "./*.cpp")

# 只遍历当前目录
file(GLOB  SRC_DIR_LIST "./*.cpp")
message("base---${PROJECT_SOURCE_DIR}")



add_library(
        base 
        SHARED
        ${SRC_DIR_LIST}									
) 

target_include_directories (
  base PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/spdlog/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory (tests) # 指定 编译子目录


