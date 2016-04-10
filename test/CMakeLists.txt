file(GLOB Tests_SRC "*.cpp")
add_executable(Test ${Tests_SRC})
target_link_libraries(Test everdb)
add_test(Test Test)

add_subdirectory(unit)
