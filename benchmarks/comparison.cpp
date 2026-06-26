#include "lru_cache.h"
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <vector>
#include <iomanip>

// Thread-safe wrapper around a plain std::unordered_map for comparison
template <typename K, typename V>
class ThreadSafeUnorderedMap {
public:
    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        map_[key] = value;
    }

    bool get(const K& key, V& out_value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it == map_.end()) {
            return false;
        }
        out_value = it->second;
        return true;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.size();
    }

private:
    std::unordered_map<K, V> map_;
    mutable std::mutex mutex_;
};

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "LRUCache vs Thread-Safe std::unordered_map Baseline" << std::endl;
    std::cout << "====================================================" << std::endl;

    const int NUM_OPS = 1000000; // 1 million operations
    const size_t capacity = 50000;

    LRUCache<int, int> lru_cache(capacity);
    ThreadSafeUnorderedMap<int, int> plain_map;

    std::cout << "Running baseline comparison (1 million operations each)..." << std::endl;

    // 1. Insertion speed test
    auto start_lru_put = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPS; ++i) {
        lru_cache.put(i, i);
    }
    auto end_lru_put = std::chrono::high_resolution_clock::now();
    double lru_put_ms = std::chrono::duration<double, std::milli>(end_lru_put - start_lru_put).count();

    auto start_map_put = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPS; ++i) {
        plain_map.put(i, i);
    }
    auto end_map_put = std::chrono::high_resolution_clock::now();
    double map_put_ms = std::chrono::duration<double, std::milli>(end_map_put - start_map_put).count();

    // 2. Lookup speed test
    auto start_lru_get = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPS; ++i) {
        int val = 0;
        lru_cache.get(i, val);
    }
    auto end_lru_get = std::chrono::high_resolution_clock::now();
    double lru_get_ms = std::chrono::duration<double, std::milli>(end_lru_get - start_lru_get).count();

    auto start_map_get = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_OPS; ++i) {
        int val = 0;
        plain_map.get(i, val);
    }
    auto end_map_get = std::chrono::high_resolution_clock::now();
    double map_get_ms = std::chrono::duration<double, std::milli>(end_map_get - start_map_get).count();

    // Memory overhead calculation (Qualitative details)
    // std::list node has 2 pointers (prev, next) and the std::pair<K, V>.
    // std::unordered_map buckets have pointers to node, plus hash collisions buckets.
    // LRU Cache uses BOTH list and map, so its memory footprint is higher.
    size_t list_node_size = sizeof(std::pair<int, int>) + 2 * sizeof(void*);
    size_t map_node_size = sizeof(void*) + sizeof(int) + sizeof(void*); // approx node allocation overhead
    size_t lru_per_element_estimate = list_node_size + map_node_size;
    size_t map_per_element_estimate = sizeof(std::pair<int, int>) + sizeof(void*);

    std::cout << "\nOperation      | LRUCache (ms) | plain std::unordered_map (ms) | Factor" << std::endl;
    std::cout << "-----------------------------------------------------------------------" << std::endl;
    std::cout << "Insert (Put)   | " 
              << std::left << std::setw(13) << std::fixed << std::setprecision(1) << lru_put_ms << " | "
              << std::left << std::setw(30) << map_put_ms << " | "
              << std::fixed << std::setprecision(2) << (lru_put_ms / map_put_ms) << "x slower" << std::endl;
              
    std::cout << "Lookup (Get)   | "
              << std::left << std::setw(13) << std::fixed << std::setprecision(1) << lru_get_ms << " | "
              << std::left << std::setw(30) << map_get_ms << " | "
              << std::fixed << std::setprecision(2) << (lru_get_ms / map_get_ms) << "x slower" << std::endl;

    std::cout << "\nMemory Overhead (Estimated per key-value element):" << std::endl;
    std::cout << "- LRUCache: ~" << lru_per_element_estimate << " bytes per element (std::list node + hash table index + iterator pointer)" << std::endl;
    std::cout << "- plain std::unordered_map: ~" << map_per_element_estimate << " bytes per element" << std::endl;
    std::cout << "- LRU Cache memory amplification: ~" << (double)lru_per_element_estimate / map_per_element_estimate << "x baseline map size." << std::endl;

    return 0;
}
