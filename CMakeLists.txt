cmake_minimum_required(VERSION 3.20)

set( PROJECT_NAME "webserver" )
set(CMAKE_CXX_STANDARD 17)

project(${PROJECT_NAME})

# 指定头文件搜索路径（）
# target_include_directories (${CMAKE_CURRENT_SOURCE_DIR}/base  )

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog )


# set(base_DIR ${CMAKE_CURRENT_SOURCE_DIR}/base)
# find_package(base REQUIRED)

# set(spdlog_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog)
# find_package(spdlog REQUIRED)



add_subdirectory ("base") # 指定 编译子目录
add_subdirectory ("net") # 指定 编译子目录


set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# 指定可执行文件名及源码
add_executable(${PROJECT_NAME}  main.cpp)

# 指定链接库
target_link_libraries(
  ${PROJECT_NAME} PUBLIC 
  net 
  ) 

# 指定头文件
target_include_directories (
  ${PROJECT_NAME} PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog/include
  # ${CMAKE_CURRENT_SOURCE_DIR}/base/include
  
)





# ADD_SUBDIRECTORY(base/)




# include_directories(   													
#         base/
# )
