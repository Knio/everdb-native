add_subdirectory(unit)

file(GLOB Tests_SRC "*.cpp")
add_executable(test_integration ${Tests_SRC})
target_link_libraries(test_integration everdb)
add_test(test_integration test_integration)
