cmake_minimum_required(VERSION 3.10)


project(SIMPLE_ECHO)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER clang)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
add_subdirectory(coroutine/src)

add_executable(echo_server server.c utils.h)
add_executable(echo_client client.c utils.h)

target_link_libraries(echo_server rt pthread src)
target_link_libraries(echo_client rt pthread src)

