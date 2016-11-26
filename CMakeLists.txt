cmake_minimum_required(VERSION 2.6)
project(everdb-native)

SET(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_C_FLAGS "-std=c99 -fms-extensions -O3")
SET(CMAKE_C_FLAGS_DEBUG "-std=c99 -g -Wall --coverage")

SET(CMAKE_CXX_FLAGS "-fms-extensions -fpermissive -O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -Wall --coverage")

SET(CMAKE_LINKER_FLAGS_DEBUG "--coverage")

add_library(
    everdb
    src/io.c
    src/page.c
    src/array.c
    src/hash.c
    src/txn.c
    src/edb.c
)

enable_testing()
add_subdirectory(test)
