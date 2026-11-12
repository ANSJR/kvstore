
#include "kv/ExpiryHeap.hpp"
#include <cassert>

namespace kv {
void ExpiryHeap::push(Node* n) {
    expiryHeap.push_back(HeapNode{*n->second.expiresAt, n});
    sift_up(expiryHeap.size() - 1);
}
// Updates a changed Node like a change in time
void ExpiryHeap::update(Node* n);

void ExpiryHeap::remove(Node* n);
// Returns the earliest expiration and removes it
Node* ExpiryHeap::pop_min();

// Soonest deadline
TimePoint ExpiryHeap::min_deadline() const;
// Returns true if empty
bool ExpiryHeap::empty() const { return expiryHeap.empty(); }
// Returns number of expiring keys
std::size_t ExpiryHeap::size() const { return expiryHeap.size(); }

// Removes everything
void ExpiryHeap::clear();

// For debugging if heap and hash table disagree
bool ExpiryHeap::check_invariants() const; 

// The point where any value is placed in HeapNode
void ExpiryHeap::place(std::size_t i, HeapNode s) {
    expiryHeap[i] = s;
    s.node->second.heapSlot = i;
}

void ExpiryHeap::sift_up(std::size_t i) {
    HeapNode moving = expiryHeap[i];
    while (i > 0 && expiryHeap[parent(i)].at > moving.at) {
        place(i, expiryHeap[parent(i)]);
        i = parent(i);
    }
    place(i, moving);
}
void ExpiryHeap::sift_down(std::size_t i) {
    assert(i < expiryHeap.size());
    HeapNode moving = expiryHeap[i];
    while (true) {
        std::size_t smallest = i; // where the smallest is
        std::size_t l = left(i);
        std::size_t r = right(i);
        TimePoint best = moving.at; // whats the smallest's deadline

        if (l < expiryHeap.size() && expiryHeap[l].at < best) { smallest = l; best = expiryHeap[l].at; }
        if (r < expiryHeap.size() && expiryHeap[r].at < best) { smallest = r; best = expiryHeap[r].at; }
        if (smallest == i) break;

        place(i, expiryHeap[smallest]);
        i = smallest;
    }
    place(i, moving);
}

}