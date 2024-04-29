cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(asm_sim)

aux_source_directory(./src SRC_LIST)
aux_source_directory(./src/impl SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE ./include)
