cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

# 指定可执行文件名及源码
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

add_subdirectory (http) # 指定 编译子目录


# 宏定义 add_test_exec
macro (add_exec exec_name)
    add_executable ("${exec_name}" "${exec_name}.cpp")
    # target_include_directories( "${exec_name}" "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/spdlog/include" )
    target_link_libraries ("${exec_name}" net)
endmacro (add_exec)

add_exec(echo)
add_exec(banch)





# target_include_directories (
#     echo PUBLIC 
#     ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/spdlog/include
# #   ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# 指定链接库
# target_link_libraries(
#     echo PUBLIC
#     net 
#   ) 







