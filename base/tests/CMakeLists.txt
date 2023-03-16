
# 指定可执行文件名及源码
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../tests")

add_executable(test_btye_stream  test_btye_stream.cpp)
target_link_libraries(test_btye_stream base ) 

add_executable(test_buffer_fd_write  test_buffer_fd_write.cpp)
target_link_libraries(test_buffer_fd_write base ) 






