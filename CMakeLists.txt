cmake_minimum_required(VERSION 3.15)
project(LRU_Cache_CPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# LRU Cache interface library
add_library(lru_cache INTERFACE)
target_include_directories(lru_cache INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Threading support
find_package(Threads REQUIRED)

# 1. Functional Test
add_executable(test_lru tests/test_lru.cpp)
target_link_libraries(test_lru PRIVATE lru_cache Threads::Threads)

# 2. Stress Test
add_executable(stress_test tests/stress_test.cpp)
target_link_libraries(stress_test PRIVATE lru_cache Threads::Threads)

# 3. Benchmark
add_executable(benchmark benchmarks/benchmark.cpp)
target_link_libraries(benchmark PRIVATE lru_cache Threads::Threads)

# 4. Comparison
add_executable(comparison benchmarks/comparison.cpp)
target_link_libraries(comparison PRIVATE lru_cache Threads::Threads)
