# 🗄️ Thread-Safe LRU Cache in C++

A generic, high-performance **LRU (Least Recently Used) Cache** implemented in modern C++. Achieves **O(1)** time complexity for both `get` and `put` operations, with built-in **thread safety** via `std::mutex`.

---

## ✨ Features

- ⚡ **O(1) Performance** — Both reads and writes run in constant time
- 🔒 **Thread-Safe** — All operations protected by `std::mutex` for safe concurrent access
- 🧩 **Generic / Templated** — Works with any key-value types (`int`, `string`, custom structs, etc.)
- 📦 **Header-Only** — Just drop `lru_cache.h` into your project and go
- 🛡️ **Input Validation** — Throws `std::invalid_argument` for zero-capacity construction

---

## 📁 Project Structure

```
lru-cache-cpp/
├── lru_cache.h          # Header-only LRU Cache implementation
├── test_lru_cache.cpp   # Functional test suite
├── CMakeLists.txt       # CMake build configuration
└── walkthrough.md       # Detailed implementation walkthrough
```

---

## 🧠 How It Works

The O(1) complexity is achieved by combining two data structures:

| Structure | Role |
|---|---|
| `std::list<pair<K, V>>` | Doubly-linked list tracking usage order. MRU at front, LRU at back. |
| `std::unordered_map<K, list::iterator>` | Hash map for O(1) key lookup, pointing directly into the list. |

### `get(key, out_value)`
1. Look up the key in the hash map — **O(1)**
2. If found, move the item to the front using `std::list::splice` — **O(1)**
3. Return the value via output reference

### `put(key, value)`
1. If key exists → update value and move to front — **O(1)**
2. If key is new and cache is full → evict the item at the **back** (LRU), remove from map — **O(1)**
3. Insert new item at the front and update the map — **O(1)**

---

## 🚀 Quick Start

### Requirements
- C++11 or later
- CMake 3.10+ (optional, for building the test suite)

### Usage

```cpp
#include "lru_cache.h"

int main() {
    LRUCache<int, std::string> cache(3); // capacity = 3

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    std::string value;
    if (cache.get(1, value)) {
        std::cout << value << std::endl; // "one"
    }

    // Cache is full; adding key 4 evicts the LRU item (key 2)
    cache.put(4, "four");

    std::cout << cache.get(2, value); // false — evicted!

    return 0;
}
```

### Build & Run Tests

```bash
# Clone the repo
git clone https://github.com/Ayush59699/lru-cache-cpp.git
cd lru-cache-cpp

# Build with CMake
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
./lru_cache_tests
```

---

## 🔌 API Reference

```cpp
LRUCache<K, V>(size_t capacity)
```
Constructs a cache with the given capacity. Throws `std::invalid_argument` if capacity is 0.

```cpp
bool get(const K& key, V& out_value)
```
Retrieves the value for `key`. Returns `true` on hit (marks as MRU), `false` on miss.

```cpp
void put(const K& key, const V& value)
```
Inserts or updates a key-value pair. Evicts the LRU item if at capacity.

```cpp
size_t size() const
```
Returns the current number of items in the cache.

```cpp
void clear()
```
Removes all items from the cache.

---

## 🧪 Tests

The test suite in [`test_lru_cache.cpp`](test_lru_cache.cpp) covers:

- ✅ Basic `put` and `get` operations
- ✅ LRU eviction policy correctness
- ✅ Cache hit and miss behavior
- ✅ Update of existing keys

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).
