cmake_minimum_required(VERSION 3.20)

set( PROJECT_NAME "muduo" )
set(CMAKE_CXX_STANDARD 17)
# 设置优化等, 还有 -fPIC 的问题
# set( CXX_FLAG... )



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")  # Inhibit all warning messages.
set(CXX_FLAGS
    -g
    # -DVALGRIND
    # -DMUDUO_STD_STRING
    -DCHECK_PTHREAD_RETURN_VALUE
    -D_FILE_OFFSET_BITS=64
    -Wall
    -Wextra
    # -Werror
    # -Wconversion
    # -Wno-unused-parameter # 未使用参数
    # -Wold-style-cast
    -Wno-multichar
    -Wno-unused-function
    -Wno-reorder
    -Woverloaded-virtual
    -Wpointer-arith
    # -Wshadow
    -Wwrite-strings
    -march=native
    # -MMD
    -std=c++0x
    -rdynamic

    -fPIC
)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")




project(${PROJECT_NAME})

# 设置 bin 目录
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
# 设置Lib 目录
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# 设置头文件目录
set( INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )

# 设置第三方目录
set(3rd_PATH "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty")


# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog )


# set(base_DIR ${CMAKE_CURRENT_SOURCE_DIR}/base)
# find_package(base REQUIRED)

# set(spdlog_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog)
# find_package(spdlog REQUIRED)



add_subdirectory ("src/base") # 指定 编译子目录
add_subdirectory ("src/net") # 指定 编译子目录
add_subdirectory ("example") # 指定 编译子目录



# 指定可执行文件名及源码
add_executable(${PROJECT_NAME}  main.cpp)



# 指定头文件
# 指定头文件搜索路径（）
target_include_directories (
  ${PROJECT_NAME} PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog/include
  ${INCLUDE_PATH}
)


# 指定链接库
# 没有写包未找到的逻辑
target_link_libraries(
  ${PROJECT_NAME} PUBLIC 
  net 
  ) 
  
# target_link_libraries(
#   ${PROJECT_NAME} PUBLIC 
#   base 
#   ) 
