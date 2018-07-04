cmake_minimum_required(VERSION 3.10)
project(vehicleclient C)

set(CMAKE_C_STANDARD 99)

add_executable(vehicleclient main.c myqueue.h)