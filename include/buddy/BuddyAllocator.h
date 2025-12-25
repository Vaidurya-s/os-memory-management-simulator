#pragma once

#include <cstddef>
#include <vector>
#include <list>
#include <unordered_map>

class BuddyAllocator {
public:
    explicit BuddyAllocator(std::size_t total_memory);
    std::size_t allocate(std::size_t size);
    void free_block(std::size_t addr);

    // Metrics and analysis
    std::size_t total_memory() const;
    std::size_t allocated_memory() const;
    std::size_t free_memory() const;
    std::size_t largest_free_block() const;
    double internal_fragmentation() const;

    // Invariant checks (debug / analysis)
    bool check_no_free_buddy_pairs() const;
    bool check_no_overlaps() const;

    void dump_free_lists() const;

private:
    std::size_t total_memory_;
    std::size_t max_order_;

    // free_lists_[k] holds starting addresses of free blocks of size 2^k
    std::vector<std::list<std::size_t>> free_lists_;

    // address -> order (used later for free)
    std::unordered_map<std::size_t, std::size_t> allocated_blocks_;

    static bool is_power_of_two(std::size_t x);
    static std::size_t log2_exact(std::size_t x);
};
