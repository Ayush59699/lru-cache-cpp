#include "lru_cache.h"
#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <cassert>
#include <atomic>

void run_stress_thread(LRUCache<int, int>& cache, int num_ops, int thread_id, std::atomic<bool>& error_flag) {
    // Each thread gets its own random engine to prevent contention on random generation
    std::mt19937 rng(1337 + thread_id);
    std::uniform_int_distribution<int> op_dist(0, 9); // 0-9: 70% get (0-6), 30% put (7-9)
    std::uniform_int_distribution<int> key_dist(1, 500); // Small key space to trigger lots of evictions and updates

    for (int i = 0; i < num_ops; ++i) {
        int key = key_dist(rng);
        int op_type = op_dist(rng);

        try {
            if (op_type < 7) {
                int value = 0;
                cache.get(key, value);
            } else {
                int value = key * 10;
                cache.put(key, value);
            }
        } catch (const std::exception& e) {
            std::cerr << "Thread " << thread_id << " caught exception: " << e.what() << std::endl;
            error_flag = true;
        }
    }
}

int main() {
    std::cout << "Starting Concurrency Stress Test..." << std::endl;
    std::cout << "Target: 50 threads executing 20 million operations total (400,000 ops/thread)" << std::endl;

    const size_t capacity = 100;
    LRUCache<int, int> cache(capacity);

    const int num_threads = 50;
    const int ops_per_thread = 400000;
    std::atomic<bool> error_flag(false);

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(run_stress_thread, std::ref(cache), ops_per_thread, i, std::ref(error_flag));
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Stress test finished in " << duration << " ms." << std::endl;

    // Verify consistency
    bool internal_state_valid = cache.validate_internal_state();
    size_t final_size = cache.size();

    std::cout << "--- Verification Results ---" << std::endl;
    std::cout << "Thread execution errors: " << (error_flag ? "FAILED" : "NONE") << std::endl;
    std::cout << "Internal State Consistency Check: " << (internal_state_valid ? "PASSED" : "FAILED") << std::endl;
    std::cout << "Final Cache Size: " << final_size << " (Capacity: " << capacity << ")" << std::endl;

    assert(!error_flag);
    assert(internal_state_valid);
    assert(final_size <= capacity);

    std::cout << "\nAll concurrency stress test checks PASSED successfully!" << std::endl;
    return 0;
}
