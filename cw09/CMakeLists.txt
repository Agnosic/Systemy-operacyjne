cmake_minimum_required(VERSION 2.8)
project(sysopy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(helper helper.c)
add_library(queue queue.c)

add_executable(rollercoaster rollercoaster.c)

target_link_libraries(rollercoaster helper)
target_link_libraries(rollercoaster queue)
target_link_libraries(rollercoaster -lpthread)
target_link_libraries(rollercoaster -lrt)