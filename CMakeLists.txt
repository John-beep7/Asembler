cmake_minimum_required(VERSION 3.26)
project(Asembler2)

set(CMAKE_CXX_STANDARD 17)

add_executable(Asembler2
        main.cpp
        src_gauss_jordan.cpp
        src_gauss_jordan.h
        src_gauss_jordan_asm.s

)