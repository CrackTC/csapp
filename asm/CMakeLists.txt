cmake_minimum_required(VERSION 3.5)

project(asm)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(./common)
add_subdirectory(./parse)
add_subdirectory(./sim)
