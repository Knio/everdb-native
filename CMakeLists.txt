cmake_minimum_required(VERSION 2.8)
project(everdb-native)

SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")

# SET(CMAKE_VERBOSE_MAKEFILE "ON")

SET(CMAKE_C_FLAGS "-std=c99 -fms-extensions")
SET(CMAKE_C_FLAGS_DEBUG   "-std=c99 -Wall -fms-extensions -g --coverage")
# SET(CMAKE_C_FLAGS_DEBUG   "-std=c99 -Wall -fms-extensions -g --coverage -DDEBUG")
SET(CMAKE_C_FLAGS_RELEASE "-std=c99 -Wall -fms-extensions -O2")

SET(CMAKE_CXX_FLAGS "-fms-extensions -fpermissive -O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -Wall --coverage")

SET(CMAKE_LINKER_FLAGS_DEBUG "--coverage")

SET(SRCS
    src/io.c
    src/page.c
    src/array.c
    src/hash.c
    src/btree.c
    src/freelist.c
    src/txn.c
    src/edb.c
)

add_library(everdb ${SRCS})

enable_testing()
add_subdirectory(test)



# coveralls-cmake

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

include(Coveralls)

set(COVERAGE_SRCS ${SRCS})

coveralls_setup("${COVERAGE_SRCS}" ON)
