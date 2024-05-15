project(linker LANGUAGES C)

aux_source_directory(src DIR_SRCS)

add_executable(linker ${DIR_SRCS})
target_include_directories(
  linker
  PRIVATE
  private
  public)
target_link_libraries(${PROJECT_NAME} PRIVATE common parse_lib)

add_executable(test_list test/test_list.c src/list.c)
target_include_directories(
  test_list
  PRIVATE
  private)
target_link_libraries(test_list PRIVATE common)
add_test(NAME test_list COMMAND test_list)

add_executable(test_trie test/test_trie.c src/trie.c src/list.c)
target_include_directories(
  test_trie
  PRIVATE
  private)
target_link_libraries(test_trie PRIVATE common)
add_test(NAME test_trie COMMAND test_trie)
