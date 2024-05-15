project(parse LANGUAGES C)

aux_source_directory(./src SRC_LIST)
add_library(${PROJECT_NAME}_lib ${SRC_LIST})
target_include_directories(${PROJECT_NAME}_lib PUBLIC ./include)
target_link_libraries(${PROJECT_NAME}_lib PRIVATE common)

aux_source_directory(./test TEST_LIST)
add_executable(${PROJECT_NAME} ${TEST_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE ./include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib common)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
