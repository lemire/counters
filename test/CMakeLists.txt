# Test target for bench
add_executable(test_bench test_bench.cpp)
set_target_properties(test_bench PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

# Use the interface target for include directories
target_link_libraries(test_bench PRIVATE counters::counters)

add_test(NAME bench_test COMMAND test_bench)
