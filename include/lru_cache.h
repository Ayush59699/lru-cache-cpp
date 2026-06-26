#pragma once
#include <iostream>
#include <unordered_map>
#include <list>
#include <mutex>
#include <stdexcept>
// using namespace std;



template <typename K, typename V>
class LRUCache {
public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("Capacity must be greater than 0");
        }
    }

    
    bool get(const K& key, V& out_value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it == map_.end()) {
            return false;
        }

        // Move the accessed item to the front of the list (MRU)
        list_.splice(list_.begin(), list_, it->second);
        out_value = it->second->second;
        return true;
    }

    
    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);

        if (it != map_.end()) {
            // Key exists, update value and move to front
            it->second->second = value;
            list_.splice(list_.begin(), list_, it->second);
            return;
        }

        // Key doesn't exist. Check capacity.
        if (list_.size() >= capacity_) {
            // Evict LRU item (back of the list)
            K last_key = list_.back().first;
            map_.erase(last_key);
            list_.pop_back();
        }

        // Insert new item at the front
        list_.push_front({key, value});
        map_[key] = list_.begin();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return list_.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.clear();
        list_.clear();
    }

    bool validate_internal_state() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (list_.size() > capacity_) {
            return false;
        }
        if (map_.size() != list_.size()) {
            return false;
        }
        // Verify every hashmap entry points to a valid list node
        for (const auto& [key, list_it] : map_) {
            // Find key in list by traversing, or check that it maps back to the same key
            if (list_it->first != key) {
                return false;
            }
        }
        return true;
    }

private:
    size_t capacity_;
    std::list<std::pair<K, V>> list_; // Doubly linked list to track usage order
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> map_; // Map for O(1) access
    mutable std::mutex mutex_; // Mutex for thread safety
};
