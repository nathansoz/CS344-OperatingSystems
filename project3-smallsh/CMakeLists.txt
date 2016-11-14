cmake_minimum_required(VERSION 2.8.11)
project(smallsh)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES main.c)
add_executable(smallsh ${SOURCE_FILES})