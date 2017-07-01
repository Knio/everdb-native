add_subdirectory(unit)

file(GLOB Tests_SRC "test*.cpp")
add_executable(test_integration ${Tests_SRC})
target_link_libraries(test_integration everdb)
add_test(test_integration test_integration)

add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark everdb)

