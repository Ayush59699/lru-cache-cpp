#include "lru_cache.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <atomic>

// A struct to hold the metrics from a benchmark run
struct BenchmarkResult {
    int threads;
    long long total_ops;
    double time_ms;
    double throughput; // ops/sec
    double p50_ns;
    double p95_ns;
    double p99_ns;
};

// Thread function for performance benchmarking
void run_benchmark_thread(LRUCache<int, int>& cache, 
                          long long ops_per_thread, 
                          int thread_id, 
                          std::vector<double>& local_latencies, 
                          bool sample_latency) {
    std::mt19937 rng(42 + thread_id);
    std::uniform_int_distribution<int> op_dist(0, 9); // 0-6: get (70%), 7-9: put (30%)
    std::uniform_int_distribution<int> key_dist(1, 100000); // 100k key space

    // If sampling, we sample a subset of operations to avoid excessive memory allocation
    const long long sample_interval = sample_latency ? std::max(1LL, ops_per_thread / 20000) : 0; 
    
    for (long long i = 0; i < ops_per_thread; ++i) {
        int key = key_dist(rng);
        int op_type = op_dist(rng);

        if (sample_latency && sample_interval > 0 && i % sample_interval == 0) {
            auto start = std::chrono::high_resolution_clock::now();
            if (op_type < 7) {
                int val = 0;
                cache.get(key, val);
            } else {
                cache.put(key, i);
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::nano> lat = end - start;
            local_latencies.push_back(lat.count());
        } else {
            if (op_type < 7) {
                int val = 0;
                cache.get(key, val);
            } else {
                cache.put(key, i);
            }
        }
    }
}

// Thread-safe simulator for Hit Ratio using Zipf-like workload
void run_hit_ratio_thread(LRUCache<int, int>& cache, 
                          long long ops_per_thread, 
                          int thread_id, 
                          std::atomic<long long>& global_hits, 
                          std::atomic<long long>& global_misses) {
    std::mt19937 rng(99 + thread_id);
    std::uniform_int_distribution<int> type_dist(0, 99); // 80% hot, 20% cold
    std::uniform_int_distribution<int> hot_dist(1, 200);   // Hot keys (200 keys)
    std::uniform_int_distribution<int> cold_dist(201, 100000); // Cold keys (many keys)

    long long hits = 0;
    long long misses = 0;

    for (long long i = 0; i < ops_per_thread; ++i) {
        int key = 0;
        if (type_dist(rng) < 80) {
            key = hot_dist(rng);
        } else {
            key = cold_dist(rng);
        }

        int val = 0;
        if (cache.get(key, val)) {
            hits++;
        } else {
            misses++;
            cache.put(key, 1); // populate cache
        }
    }
    global_hits += hits;
    global_misses += misses;
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "LRU Cache Performance Benchmarking & Hit Ratio Suite" << std::endl;
    std::cout << "====================================================" << std::endl;

    const long long TOTAL_OPS = 10000000; // 10 million ops
    const size_t CACHE_CAPACITY = 50000;  // Large capacity for throughput benchmarks
    std::vector<int> thread_configs = {1, 2, 4, 8, 16, 32};
    std::vector<BenchmarkResult> results;

    for (int num_threads : thread_configs) {
        LRUCache<int, int> cache(CACHE_CAPACITY);
        long long ops_per_thread = TOTAL_OPS / num_threads;

        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        
        // Setup local latency arrays to gather latency distribution samples without lock contention on latency tracking
        std::vector<std::vector<double>> all_latencies(num_threads);
        for (int i = 0; i < num_threads; ++i) {
            all_latencies[i].reserve(30000);
        }

        std::cout << "Running benchmark with " << num_threads << " thread(s)..." << std::endl;

        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(run_benchmark_thread, 
                                 std::ref(cache), 
                                 ops_per_thread, 
                                 i, 
                                 std::ref(all_latencies[i]), 
                                 true);
        }

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end_time - start_time;

        // Flatten and compile all latency samples
        std::vector<double> combined_latencies;
        for (const auto& local : all_latencies) {
            combined_latencies.insert(combined_latencies.end(), local.begin(), local.end());
        }
        std::sort(combined_latencies.begin(), combined_latencies.end());

        double p50 = 0, p95 = 0, p99 = 0;
        if (!combined_latencies.empty()) {
            p50 = combined_latencies[combined_latencies.size() * 0.50];
            p95 = combined_latencies[combined_latencies.size() * 0.95];
            p99 = combined_latencies[combined_latencies.size() * 0.99];
        }

        double throughput_mops = (double)TOTAL_OPS / (duration.count() / 1000.0);

        results.push_back({
            num_threads,
            TOTAL_OPS,
            duration.count(),
            throughput_mops,
            p50,
            p95,
            p99
        });
    }

    // Print Performance Table
    std::cout << "\nThreads | Total Ops | Time(ms) | Throughput(ops/sec) | P50(ns) | P95(ns) | P99(ns)" << std::endl;
    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    for (const auto& r : results) {
        std::cout << std::left << std::setw(7) << r.threads << " | "
                  << std::left << std::setw(9) << "10M" << " | "
                  << std::left << std::setw(8) << std::fixed << std::setprecision(1) << r.time_ms << " | "
                  << std::left << std::setw(19) << (long long)r.throughput << " | "
                  << std::left << std::setw(7) << (int)r.p50_ns << " | "
                  << std::left << std::setw(7) << (int)r.p95_ns << " | "
                  << (int)r.p99_ns << std::endl;
    }

    // CSV Export
    std::string csv_path = "../results/benchmark_results.csv";
    std::ofstream csv_file(csv_path);
    if (csv_file.is_open()) {
        csv_file << "threads,total_ops,time_ms,throughput,p50,p95,p99\n";
        for (const auto& r : results) {
            csv_file << r.threads << ","
                     << r.total_ops << ","
                     << std::fixed << std::setprecision(3) << r.time_ms << ","
                     << (long long)r.throughput << ","
                     << (int)r.p50_ns << ","
                     << (int)r.p95_ns << ","
                     << (int)r.p99_ns << "\n";
        }
        std::cout << "\nBenchmark results successfully written to: " << csv_path << std::endl;
    } else {
        std::cerr << "\nError: Could not open file for writing CSV: " << csv_path << std::endl;
    }

    // Cache Hit Ratio Simulation
    std::cout << "\n====================================================" << std::endl;
    std::cout << "Cache Hit Ratio Workload Simulation" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    const size_t sim_capacity = 1000;
    const long long sim_requests = 5000000;
    const int sim_threads = 4;
    
    std::cout << "Capacity: " << sim_capacity << ", Requests: 5M, Concurrency: " << sim_threads << " threads" << std::endl;
    std::cout << "Workload profile: 80% repeated hot-keys, 20% random cold-keys" << std::endl;

    LRUCache<int, int> sim_cache(sim_capacity);
    std::atomic<long long> global_hits(0);
    std::atomic<long long> global_misses(0);

    std::vector<std::thread> s_threads;
    s_threads.reserve(sim_threads);
    long long reqs_per_thread = sim_requests / sim_threads;

    for (int i = 0; i < sim_threads; ++i) {
        s_threads.emplace_back(run_hit_ratio_thread, 
                               std::ref(sim_cache), 
                               reqs_per_thread, 
                               i, 
                               std::ref(global_hits), 
                               std::ref(global_misses));
    }

    for (auto& t : s_threads) {
        if (t.joinable()) t.join();
    }

    double hit_ratio = (double)global_hits / (global_hits + global_misses) * 100.0;
    std::cout << "Cache Hit Ratio: " << std::fixed << std::setprecision(1) << hit_ratio << "%" << std::endl;
    std::cout << "Hits: " << global_hits << ", Misses: " << global_misses << std::endl;

    return 0;
}
