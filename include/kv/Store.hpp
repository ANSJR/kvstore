#pragma once

#include <cstddef>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "kv/Entry.hpp"
#include "kv/ExpiryHeap.hpp"

namespace kv {

class Store {
public:
    Store() = default;

    // Delete to prevent misuse
    Store(const Store&) = delete;
    Store& operator=(const Store&) = delete;

    // Stores key into the hash table
    void set(std::string key, std::string value, std::optional<TimePoint> expiresAt = std::nullopt);
    // Deletes key into the hash table
    bool del(const std::string& key, TimePoint now = Clock::now());

    // Removes expired keys from heap
    std::size_t sweep(TimePoint now = Clock::now());

    // Flushes everything
    void clear();

    // Looks up key giving value or "nullopt" representing no value
    std::optional<std::string> get(const std::string& key, TimePoint now = Clock::now()) const;

    // Holds the next earliest expiration with "nullopt" empty
    std::optional<TimePoint> nextExpiry() const;

    // Raw entry count — may briefly include keys past their deadline 
    std::size_t size() const;

private:
    // checks if (entry.expiration <= now)
    static bool isExpired(const Entry& e, TimePoint now);

    mutable std::shared_mutex storeMutex;  // guards entryIndex AND expiryIndex as one unit
    std::unordered_map<std::string, Entry> entryIndex;
    ExpiryHeap expiryIndex;
};

}