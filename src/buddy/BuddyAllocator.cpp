#include "buddy/BuddyAllocator.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>

bool BuddyAllocator::is_power_of_two(std::size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

std::size_t BuddyAllocator::log2_exact(std::size_t x) {
    std::size_t k = 0;
    while ((static_cast<std::size_t>(1) << k) < x) {
        ++k;
    }
    return k;
}

BuddyAllocator::BuddyAllocator(std::size_t total_memory)
    : total_memory_(total_memory), next_id_(1) {

    if (!is_power_of_two(total_memory_)) {
        throw std::invalid_argument(
            "BuddyAllocator requires total memory to be a power of two");
    }

    max_order_ = log2_exact(total_memory_);
    free_lists_.resize(max_order_ + 1);

    // Entire memory starts as one free block
    free_lists_[max_order_].push_back(0);
}

void BuddyAllocator::dump_free_lists() const {
    std::cout << "Free Blocks by Order:\n";
    for (std::size_t order = 0; order <= max_order_; ++order) {
        std::size_t block_size = static_cast<std::size_t>(1) << order;
        
        if (!free_lists_[order].empty()) {
            std::cout << "Order " << order
                      << " (size " << block_size << "): ";

            for (std::size_t addr : free_lists_[order]) {
                std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0') 
                          << addr << std::dec << " ";
            }
            std::cout << "\n";
        }
    }
    
    // Also show allocated blocks
    if (!allocated_blocks_.empty()) {
        std::cout << "\nAllocated Blocks:\n";
        for (const auto& [addr, order] : allocated_blocks_) {
            std::size_t size = static_cast<std::size_t>(1) << order;
            std::cout << "[0x" << std::hex << std::setw(4) << std::setfill('0')
                      << addr << " - 0x" << std::setw(4) << std::setfill('0')
                      << (addr + size - 1) << std::dec << "] USED (size=" << size << ")\n";
        }
    }
}


static std::size_t next_power_of_two(std::size_t x) {
    if (x == 0) return 1;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    if (sizeof(std::size_t) == 8) {
        x |= x >> 32;
    }
    return x + 1;
}


std::size_t BuddyAllocator::allocate_buddy(std::size_t size) {
    if (size == 0 || size > total_memory_) {
        return static_cast<std::size_t>(-1);
    }

    std::size_t rounded_size = next_power_of_two(size);
    std::size_t target_order = log2_exact(rounded_size);

    if (target_order > max_order_) {
        return static_cast<std::size_t>(-1);
    }

    // Find the smallest order >= target_order with a free block
    std::size_t current_order = target_order;
    while (current_order <= max_order_ &&
           free_lists_[current_order].empty()) {
        ++current_order;
    }

    if (current_order > max_order_) {
        return static_cast<std::size_t>(-1);
    }

    // Take a block from the higher order
    std::size_t addr = free_lists_[current_order].front();
    free_lists_[current_order].pop_front();

    // Split until we reach the target order
    while (current_order > target_order) {
        --current_order;
        std::size_t buddy_addr = addr + (static_cast<std::size_t>(1) << current_order);
        free_lists_[current_order].push_front(buddy_addr);
    }

    allocated_blocks_[addr] = target_order;
    return addr;
}


void BuddyAllocator::free_buddy(std::size_t addr) {
    auto it = allocated_blocks_.find(addr);
    if (it == allocated_blocks_.end()) {
        return;
    }

    std::size_t order = it->second;
    allocated_blocks_.erase(it);

    std::size_t current_addr = addr;
    std::size_t current_order = order;

    while (current_order < max_order_) {
        std::size_t buddy_addr = current_addr ^ (static_cast<std::size_t>(1) << current_order);

        auto& free_list = free_lists_[current_order];
        auto buddy_it = std::find(free_list.begin(), free_list.end(), buddy_addr);

        if (buddy_it == free_list.end()) {
            break;
        }

        free_list.erase(buddy_it);
        current_addr = std::min(current_addr, buddy_addr);
        ++current_order;
    }

    free_lists_[current_order].push_front(current_addr);
}

std::size_t BuddyAllocator::total_memory() const {
    return total_memory_;
}

std::size_t BuddyAllocator::allocated_memory() const {
    std::size_t sum = 0;
    for (const auto& [addr, order] : allocated_blocks_) {
        sum += static_cast<std::size_t>(1) << order;
    }
    return sum;
}

std::size_t BuddyAllocator::free_memory() const {
    return total_memory_ - allocated_memory();
}

std::size_t BuddyAllocator::largest_free_block() const {
    for (std::size_t order = max_order_; order > 0; --order) {
        if (!free_lists_[order].empty()) {
            return static_cast<std::size_t>(1) << order;
        }
    }
    return free_lists_[0].empty() ? 0 : 1;
}

double BuddyAllocator::internal_fragmentation() const {
    std::size_t allocated = allocated_memory();
    if (allocated == 0) {
        return 0.0;
    }

    std::size_t requested = 0;
    for (const auto& [addr, order] : allocated_blocks_) {
        requested += static_cast<std::size_t>(1) << order;
    }

    return static_cast<double>(allocated - requested) /
           static_cast<double>(allocated);
}


bool BuddyAllocator::check_no_free_buddy_pairs() const {
    for (std::size_t order = 0; order < max_order_; ++order) {
        for (std::size_t addr : free_lists_[order]) {
            std::size_t buddy =
                addr ^ (static_cast<std::size_t>(1) << order);
            for (std::size_t other : free_lists_[order]) {
                if (other == buddy) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool BuddyAllocator::check_no_overlaps() const {
    std::unordered_set<std::size_t> seen;
    for (std::size_t order = 0; order <= max_order_; ++order) {
        std::size_t size = static_cast<std::size_t>(1) << order;
        for (std::size_t addr : free_lists_[order]) {
            for (std::size_t i = 0; i < size; ++i) {
                if (!seen.insert(addr + i).second) {
                    return false;
                }
            }
        }
    }
    return true;
}

// IAllocator interface implementation
int BuddyAllocator::allocate(std::size_t size) {
    std::size_t addr = allocate_buddy(size);
    if (addr == static_cast<std::size_t>(-1)) {
        return -1;
    }
    
    int id = next_id_++;
    id_to_addr_[id] = addr;
    addr_to_id_[addr] = id;
    return id;
}

void BuddyAllocator::free_block(int id) {
    auto it = id_to_addr_.find(id);
    if (it == id_to_addr_.end()) {
        return;
    }
    
    std::size_t addr = it->second;
    id_to_addr_.erase(it);
    addr_to_id_.erase(addr);
    
    free_buddy(addr);
}

std::size_t BuddyAllocator::used_memory() const {
    return allocated_memory();
}

void BuddyAllocator::dump() const {
    dump_free_lists();
}

const char* BuddyAllocator::allocator_name() const {
    return "Buddy System";
}

