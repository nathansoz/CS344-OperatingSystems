cmake_minimum_required(VERSION 2.8.11)
project(smallsh)

set(CMAKE_C_FLAGS "-g ${CMAKE_C_FLAGS} -std=gnu99")

set(SOURCE_FILES main.c command.c)
add_executable(smallsh ${SOURCE_FILES})