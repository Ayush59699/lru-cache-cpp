# Thread-Safe LRU Cache Implementation Plan

Implementing a generic LRU (Least Recently Used) cache with $O(1)$ complexity for both `get` and `put` operations, with thread safety ensured using C++ standard synchronization primitives.

## User Review Required

> [!IMPORTANT]
> - The $O(1)$ complexity for `get` is achieved by using a hash map for lookup and a doubly linked list for eviction tracking.
> - **Thread Safety**: I will use `std::mutex` for simple, robust thread safety. Note that even `get` operations modify the underlying list (to move elements to the front), so they require exclusive access (or at least more complex locking than simple read/write locks).

## Proposed Changes

### Core Implementation
We will use a header-only template class for maximum flexibility.

#### [NEW] [lru_cache.h](file:///c:/Users/Hp/OneDrive/Desktop/ANTIGRATIVY TESTS/LRU_CACHE_C++/lru_cache.h)
- Class `LRUCache<K, V>`
- Private members:
    - `std::list<std::pair<K, V>> cache_list`: Stores key-value pairs.
    - `std::unordered_map<K, Iterator> cache_map`: Maps keys to list iterators.
    - `mutable std::mutex mtx`: For thread synchronization.
    - `size_t capacity`: Maximum items allowed.
- Public Methods:
    - `V get(const K& key)`: Retrieves value and updates position.
    - `void put(const K& key, const V& value)`: Inserts/updates value and handles eviction.

### Project Setup

#### [NEW] [CMakeLists.txt](file:///c:/Users/Hp/OneDrive/Desktop/ANTIGRATIVY TESTS/LRU_CACHE_C++/CMakeLists.txt)
- Standard CMake configuration for C++20.

### Testing

#### [NEW] [test_lru_cache.cpp](file:///c:/Users/Hp/OneDrive/Desktop/ANTIGRATIVY TESTS/LRU_CACHE_C++/test_lru_cache.cpp)
- Unit tests for basic functional requirements.
- Concurrency tests using `std::thread` to verify thread safety.

## Verification Plan

### Automated Tests
- Build using CMake.
- Run `test_lru_cache` executable.
- Validate that capacity is respected and LRU items are evicted.
- Verify no deadlocks or race conditions during concurrent access.

### Manual Verification
- Review code for $O(1)$ guarantees (lookup in map, move to list front).
