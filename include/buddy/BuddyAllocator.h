#pragma once

#include "../allocator/IAllocator.h"
#include <cstddef>
#include <vector>
#include <list>
#include <unordered_map>

class BuddyAllocator : public IAllocator {
public:
    explicit BuddyAllocator(std::size_t total_memory);
    
    // Original buddy-specific methods (return addresses)
    std::size_t allocate_buddy(std::size_t size);
    void free_buddy(std::size_t addr);

    // IAllocator interface implementation (uses block IDs)
    int allocate(std::size_t size) override;
    void free_block(int id) override;
    std::size_t total_memory() const override;
    std::size_t used_memory() const override;
    std::size_t free_memory() const override;
    std::size_t largest_free_block() const override;
    void dump() const override;
    const char* allocator_name() const override;

    // Metrics and analysis
    std::size_t allocated_memory() const;
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
    
    // Map block IDs to addresses for interface compliance
    std::unordered_map<int, std::size_t> id_to_addr_;
    std::unordered_map<std::size_t, int> addr_to_id_;
    int next_id_;

    static bool is_power_of_two(std::size_t x);
    static std::size_t log2_exact(std::size_t x);
};
