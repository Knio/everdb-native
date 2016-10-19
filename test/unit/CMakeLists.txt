add_executable(TestPage test_page.cpp ${everdb-native_SOURCE_DIR}/src/page.c)
add_executable(TestMemHash test_mem_hash.cpp ${everdb-native_SOURCE_DIR}/src/mem_hash.h)

add_test(TestPage TestPage)
add_test(TestMemHash TestMemHash)
