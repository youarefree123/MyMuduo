
# 指定可执行文件名及源码
# set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../tests")

# 宏定义 add_test_exec


set( GTEST_LIB gtest gtest_main )


macro (add_test_exec exec_name)
    add_executable ("${exec_name}" "${exec_name}.cpp")
    target_link_libraries ("${exec_name}" base  ${GTEST_LIB} )
endmacro (add_test_exec)

add_test_exec (test_buffer)
add_test_exec (test_log)
add_test_exec (test_timestamp)
add_test_exec (test_thread)







