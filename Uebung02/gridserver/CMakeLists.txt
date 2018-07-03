cmake_minimum_required(VERSION 3.10)
project(gridserver C)

set(CMAKE_C_STANDARD 99)

add_executable(gridserver main.c myqueue.h driver.h grid.h)