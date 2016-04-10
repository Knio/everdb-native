cmake_minimum_required(VERSION 2.6)
project(everdb-native)

SET(CMAKE_C_FLAGS "-fms-extensions")
SET(CMAKE_C_FLAGS_DEBUG "-g -Wall --coverage") 
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fpermissive")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
SET(CMAKE_LINKER_FLAGS_DEBUG "--coverage")

add_library(everdb src/array.c src/edb.c src/hash.c
            src/page.c)

enable_testing()
add_subdirectory(test)

