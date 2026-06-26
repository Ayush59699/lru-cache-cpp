# Thread-Safe LRU Cache in C++

A generic, high-performance LRU (Least Recently Used) Cache implemented in modern C++. Achieves O(1) time complexity for both `get` and `put` operations, with built-in thread safety via `std::mutex`.

---

## Metrics and Benchmarks

The following measurements were collected on a multi-core processor running Release mode builds:

### Performance & Latency (10 Million operations: 70% get, 30% put)

| Threads | Total Ops | Time (ms) | Throughput (ops/sec) | P50 Latency (ns) | P95 Latency (ns) | P99 Latency (ns) |
|---|---|---|---|---|---|---|
| 1 | 10M | 2182.1 | 4,582,800 | 0 | 1000 | 2000 |
| 2 | 10M | 2789.7 | 3,584,604 | 0 | 1000 | 83000 |
| 4 | 10M | 2592.8 | 3,856,875 | 0 | 1000 | 151000 |
| 8 | 10M | 3310.3 | 3,020,844 | 0 | 1000 | 328000 |
| 16 | 10M | 3210.7 | 3,114,603 | 0 | 1000 | 439000 |
| 32 | 10M | 3306.1 | 3,024,743 | 0 | 1000 | 423000 |

### Cache Hit Ratio Simulation (5 Million operations, capacity = 1000)
- Workload: 80% requests to hot keys (200 keys), 20% requests to cold keys (many keys)
- **Cache Hit Ratio: 80.2%** (Hits: 4,007,780, Misses: 992,220)

### Baseline Comparison (1 Million operations: LRUCache vs mutex-guarded std::unordered_map)

| Operation | LRUCache (ms) | Thread-Safe std::unordered_map (ms) | Speed Overhead |
|---|---|---|---|
| Insert (Put) | 237.5 | 119.7 | 1.98x slower |
| Lookup (Get) | 11.1 | 9.3 | 1.19x slower |

#### Memory Footprint Comparison
- **LRUCache**: ~28 bytes per element (extra list node overhead + hash index entries + iterator pointers).
- **std::unordered_map**: ~12 bytes per element.
- **LRUCache Memory Amplification**: ~2.33x standard hashmap baseline.

---

## Features

- O(1) Performance — Both reads and writes run in constant time.
- Thread-Safe — All operations protected by `std::mutex` for safe concurrent access.
- Generic / Templated — Works with any key-value types (`int`, `string`, custom structs, etc.).
- Header-Only — Just drop `lru_cache.h` into your project and go.
- Input Validation — Throws `std::invalid_argument` for zero-capacity construction.
- Performance Benchmarking — Built-in multi-threaded throughput and latency profiling.
- Stress Testing — Exhaustive concurrent correctness validation.
- Comparative Profiling — Comparison with standard structures.

---

## Project Structure

```
lru-cache-cpp/
├── include/
│   └── lru_cache.h          # Header-only LRU Cache implementation
├── tests/
│   ├── test_lru.cpp         # Functional correctness unit tests
│   └── stress_test.cpp      # Concurrency stress test (50 threads, 20M operations)
├── benchmarks/
│   ├── benchmark.cpp        # Performance benchmarks, latency profiles & workload simulator
│   └── comparison.cpp       # Baseline comparison vs std::unordered_map
├── scripts/
│   └── run_all.sh           # Test/Benchmark execution script
├── results/
│   └── benchmark_results.csv# Saved benchmark execution CSV output
└── CMakeLists.txt           # Build configuration
```

---

## Architecture and Design

The O(1) complexity is achieved by combining two complementary data structures:

| Structure | Role |
|---|---|
| `std::list<std::pair<K, V>>` | Doubly-linked list tracking usage order. The Most Recently Used (MRU) node is at the front, and the Least Recently Used (LRU) node is at the back. |
| `std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator>` | Hash table mapping keys to corresponding list node iterators. This permits O(1) access. |

### Why list + unordered_map achieves O(1) operations:
1. **Lookup (Get)**: Look up the key in the hash map (constant time). If found, we use `std::list::splice` to move the list node to the head of the list. Since list splicing only alters pointers, moving elements within the list is a guaranteed $O(1)$ operation.
2. **Insertion (Put)**: Look up the key. If it exists, we update the value and splice it to the front ($O(1)$). If it does not exist, we check capacity: if full, we remove the back node of the list from both the list and the map ($O(1)$) and insert the new node at the front ($O(1)$).

### Thread Safety Concurrency Design:
All public APIs (`get`, `put`, `size`, `clear`, `validate_internal_state`) utilize a `std::lock_guard` with a `mutable std::mutex`. This provides mutual exclusion, ensuring only one thread can modify or read the underlying doubly-linked list and hash map at any given time. This guarantees complete thread safety, prevention of race conditions, and avoidance of data corruption under high concurrent execution.

---

## How It Works

### get(key, out_value)
1. Lock the cache mutex.
2. Look up the key in the hash map.
3. If not found, unlock and return `false`.
4. If found, splice the corresponding iterator to the front of the list.
5. Store the value in the output reference, unlock, and return `true`.

### put(key, value)
1. Lock the cache mutex.
2. Look up the key in the hash map.
3. If the key exists, update its value in the list node, splice the node to the front, and unlock.
4. If the key is new:
   - Check if size matches capacity.
   - If capacity is reached, retrieve the key of the back node (LRU node), erase it from the hash map, pop it from the list.
   - Push the new key-value pair to the front of the list, map the key to the new front iterator, and unlock.

---

## Quick Start

### Requirements
- C++17 or C++20 compiler (e.g., GCC 9+)
- CMake 3.15+

### Build & Run Everything

```bash
# Clone the repository
git clone https://github.com/Ayush59699/lru-cache-cpp.git
cd lru-cache-cpp

# Create a build directory
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release

# Run tests & benchmarks
./test_lru
./stress_test
./benchmark
./comparison
```

Alternatively, run the automated script from the root directory:
```bash
./scripts/run_all.sh
```

---

## API Reference

```cpp
LRUCache<K, V>(size_t capacity)
```
Constructs an LRU cache with the specified capacity. Throws `std::invalid_argument` if capacity is 0.

```cpp
bool get(const K& key, V& out_value)
```
Looks up the value corresponding to `key`. If found, updates its access status to MRU (front of list) and returns `true`. Otherwise returns `false`.

```cpp
void put(const K& key, const V& value)
```
Inserts or updates the key-value pair. If capacity is exceeded, evicts the LRU item.

```cpp
size_t size() const
```
Returns the count of elements currently present in the cache.

```cpp
void clear()
```
Clears all entries from the cache.

```cpp
bool validate_internal_state() const
```
Performs checks to verify map-list sizes match, capacity constraints are met, and keys align correctly.

---

## License

This project is licensed under the MIT License.
