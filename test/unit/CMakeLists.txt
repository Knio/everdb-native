add_executable(TestPage test_page.cpp ${everdb-native_SOURCE_DIR}/src/page.c)
add_executable(TestTxn test_txn.cpp ${everdb-native_SOURCE_DIR}/src/txn.c)

add_test(TestPage TestPage)
