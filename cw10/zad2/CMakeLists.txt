cmake_minimum_required(VERSION 2.8)
project(sysopy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(helper helper.c)

add_executable(server server.c)
add_executable(client client.c)

target_link_libraries(server helper)
target_link_libraries(server -lpthread)
target_link_libraries(server -lrt)
target_link_libraries(client helper)
target_link_libraries(client -lpthread)
target_link_libraries(client -lrt)
