cmake_minimum_required(VERSION 3.15)
project(LRU_Cache_CPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# LRU Cache is header-only, but we'll create an interface library for it
add_library(lru_cache INTERFACE)
target_include_directories(lru_cache INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Test executable
add_executable(test_lru_cache test_lru_cache.cpp)
target_link_libraries(test_lru_cache PRIVATE lru_cache)

# Enable threading
find_package(Threads REQUIRED)
target_link_libraries(test_lru_cache PRIVATE Threads::Threads)
