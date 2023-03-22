cmake_minimum_required(VERSION 3.20)

set( PROJECT_NAME "webserver" )
set(CMAKE_CXX_STANDARD 17)

project(${PROJECT_NAME})

# 指定头文件搜索路径（动态库需要？？）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/base)

add_subdirectory ("base") # 指定 编译子目录
add_subdirectory ("net") # 指定 编译子目录


set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# 指定可执行文件名及源码
add_executable(${PROJECT_NAME}  main.cpp)
target_link_libraries(${PROJECT_NAME} base ) 

# add_executable(test_btye_stream  test_btye_stream.cpp)
# target_link_libraries(test_btye_stream base ) 






# ADD_SUBDIRECTORY(base/)




# include_directories(   													
#         base/
# )
