cmake_minimum_required(VERSION 3.18)
project(xtra C)

set(CMAKE_C_STANDARD 99)


add_executable(xtra main.c xtra.c xtra.h xis.h)
