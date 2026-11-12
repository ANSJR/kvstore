#pragma once


#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "kv/Entry.hpp"

namespace kv {

class ExpiryHeap {
public:
    // Really just std::pair<const std::string, Entry>
    using Node = std::unordered_map<std::string, Entry>::value_type;

    // Add an entry to the heap
    void push(Node* n);
    // Updates a changed Node like a change in time
    void update(Node* n);

    void remove(Node* n);
    // Returns the earliest expiration and removes it
    Node* pop_min();

    // Soonest deadline
    TimePoint min_deadline() const;
    // Returns true if empty
    bool empty() const;
    // Returns number of expiring keys
    std::size_t size() const;

    // Removes everything
    void clear();

    // For debugging if heap and hash table disagree
    bool check_invariants() const;

private:
    struct HeapNode {
        TimePoint at;  // cached copy of *node->second.expires_at
        Node* node;  // Pointer to an entry in entryIndex 
    };
    // The point where any value is placed in HeapNode
    void place(std::size_t i, HeapNode s);

    void sift_up(std::size_t i);
    void sift_down(std::size_t i);

    static std::size_t parent(std::size_t i) { return (i - 1) / 2; }
    static std::size_t left(std::size_t i)   { return 2 * i + 1; }
    static std::size_t right(std::size_t i)  { return 2 * i + 2; }

    std::vector<HeapNode> expiryHeap;
};

}  // namespace kv