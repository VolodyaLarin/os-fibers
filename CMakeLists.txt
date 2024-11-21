cmake_minimum_required(VERSION 3.29)
project(lab01_fibers CXX C)

set(CMAKE_C_STANDARD 11)

add_executable(lab01_fibers_hello examples/hello.c fibers_lib/fibers.c
)
add_executable(lab01_fibers_pingpong examples/pingpong.c fibers_lib/fibers.c
        fibers_lib/channels.c
        fibers_lib/channels.h
)


add_executable(lab01_fibers_hello_cpp  fibers_lib/fibers.c examples/hello.cpp)
