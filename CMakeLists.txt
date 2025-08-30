cmake_minimum_required(VERSION 3.31)
project(td_game)

set(CMAKE_CXX_STANDARD 20)

add_executable(td_game main.cpp
        vec.h
        types.h
        trig.h)
