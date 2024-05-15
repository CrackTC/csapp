project(sim)

aux_source_directory(./src SRC_LIST)
aux_source_directory(./src/impl SRC_LIST)
aux_source_directory(./src/examples SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE ./include)
target_link_libraries(${PROJECT_NAME} PRIVATE common parse_lib)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/examples/add.txt
               ${CMAKE_CURRENT_BINARY_DIR}/add.txt COPYONLY)
