cmake_minimum_required(VERSION 3.26)
project(Asembler2)

enable_language(ASM)

set(CMAKE_CXX_STANDARD 17)

add_executable(Asembler2
        main.cpp
        src_gauss_jordan.cpp
        src_gauss_jordan.h
        matrix_asm.s
)