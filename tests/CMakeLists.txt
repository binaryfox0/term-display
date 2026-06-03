cmake_minimum_required(VERSION 3.10)

add_executable(static static.c)
target_link_libraries(static PRIVATE aparse term-display)
target_add_asan(static)
