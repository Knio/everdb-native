add_executable(test_page test_page.cpp ${everdb-native_SOURCE_DIR}/src/page.c)
add_executable(test_io test_io.cpp ${everdb-native_SOURCE_DIR}/src/io.c)
add_executable(test_memhash test_mem_hash.cpp ${everdb-native_SOURCE_DIR}/src/mem_hash.h)

add_test(test_io test_io)
add_test(test_page test_page)
add_test(test_memhash test_memhash)
