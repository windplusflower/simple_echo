cmake_minimum_required(VERSION 3.10)


project(SIMPLE_ECHO)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

add_executable(echo_server server.c utils.h)
add_executable(echo_client client.c utils.h)

target_link_libraries(echo_server rt pthread)
target_link_libraries(echo_client rt pthread)

