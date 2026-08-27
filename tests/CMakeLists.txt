add_executable(biosecure_tests test_main.cpp)
target_link_libraries(biosecure_tests PRIVATE biosecure_core r307)
add_test(NAME biosecure_tests COMMAND biosecure_tests)
