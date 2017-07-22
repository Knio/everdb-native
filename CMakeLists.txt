cmake_minimum_required(VERSION 2.8)
project(everdb-native)

# set(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

SET(CMAKE_C_FLAGS "-std=c11 -fms-extensions -Wall")
SET(CMAKE_CXX_FLAGS "-std=c++11 -fms-extensions -Wall -fpermissive")

SET(CMAKE_C_FLAGS_DEBUG   "-g --coverage -DDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")

SET(CMAKE_C_FLAGS_RELEASE "-O2")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

SET(CMAKE_LINKER_FLAGS_DEBUG "--coverage")

SET(SRCS
    src/io.c
    src/page.c
    src/array.c
    src/freelist.c
    src/btree.c
    src/txn.c
    src/edb.c
    # src/hash.c

    src/api/edb.c
    src/api/array.c
)

add_library(everdb ${SRCS})

enable_testing()
add_subdirectory(test)


# coveralls-cmake

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
include(Coveralls)
set(COVERAGE_SRCS ${SRCS})
coveralls_setup("${COVERAGE_SRCS}" ON)
