#include "virtual_memory/VirtualMemoryManager.h"

#include <cmath>
#include <stdexcept>
#include <cassert>
#include <limits>
#include <iostream>

static bool is_power_of_two(std::size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

VirtualMemoryManager::VirtualMemoryManager(std::size_t num_virtual_pages,
                                           std::size_t num_physical_frames,
                                           std::size_t page_size_bytes,
                                           PageReplacementPolicy policy)
    : timestamp_(0),
      page_size_(page_size_bytes),
      offset_bits_(0),
      page_table_(num_virtual_pages),
      frame_free_(num_physical_frames, true),
      page_faults_(0),
      replacement_policy_(policy)
     
{
    if (!is_power_of_two(page_size_)) {
        throw std::invalid_argument("Page size must be a power of two");
    }

    offset_bits_ = static_cast<std::size_t>(std::log2(page_size_));
}

std::size_t VirtualMemoryManager::decode_vpn(std::uint64_t virtual_address) const {
    return virtual_address >> offset_bits_;
}

std::size_t VirtualMemoryManager::decode_offset(std::uint64_t virtual_address) const {
    return virtual_address & ((1ULL << offset_bits_) - 1);
}

std::size_t VirtualMemoryManager::allocate_frame() {
    for (std::size_t i = 0; i < frame_free_.size(); ++i) {
        if (frame_free_[i]) {
            frame_free_[i] = false;
            return i;
        }
    }
    throw std::runtime_error("Out of physical frames");
}

std::uint64_t VirtualMemoryManager::translate(std::uint64_t virtual_address) {
    std::size_t vpn = decode_vpn(virtual_address);
    std::size_t offset = decode_offset(virtual_address);
    bool page_fault = false;


    if (vpn >= page_table_.size()) {
        throw std::out_of_range("Virtual address out of range");
    }

    PageTableEntry& pte = page_table_[vpn];

    if (!pte.valid) {
        page_fault = true;
        ++page_faults_;

        std::size_t frame;
        bool allocated = false;

        // Try to find a free frame
        for (std::size_t i = 0; i < frame_free_.size(); ++i) {
            if (frame_free_[i]) {
                frame_free_[i] = false;
                frame = i;
                allocated = true;
                break;
            }
        }

        if (!allocated) {
            std::size_t victim_vpn;
            if (replacement_policy_ == PageReplacementPolicy::FIFO) {
                victim_vpn = find_fifo_victim_page();
            } else {
                victim_vpn = find_lru_victim_page();
            }

            auto& victim_pte = page_table_[victim_vpn];

            frame = victim_pte.frame_number;
            victim_pte.valid = false;
        }

        pte.frame_number = frame;
        pte.valid = true;
        // pte.loaded_at = timestamp_++;
        if (replacement_policy_ == PageReplacementPolicy::FIFO) {
            pte.loaded_at = timestamp_++;
        } else if (!page_fault && replacement_policy_ == PageReplacementPolicy::LRU) {
            pte.loaded_at = timestamp_++;
        }
    }

    return pte.frame_number * page_size_ + offset;
}

std::size_t VirtualMemoryManager::page_faults() const {
    return page_faults_;
}


std::size_t VirtualMemoryManager::find_fifo_victim_page() const {
    std::size_t victim = static_cast<std::size_t>(-1);
    std::uint64_t oldest = static_cast<std::uint64_t>(-1);

    for (std::size_t i = 0; i < page_table_.size(); ++i) {
        const auto& pte = page_table_[i];
        if (pte.valid && pte.loaded_at < oldest) {
            oldest = pte.loaded_at;
            victim = i;
        }
    }

    return victim;
}


std::size_t VirtualMemoryManager::find_lru_victim_page() const {
    std::size_t victim = static_cast<std::size_t>(-1);
    std::uint64_t least_recent = static_cast<std::uint64_t>(-1);

    for (std::size_t i = 0; i < page_table_.size(); ++i) {
        const auto& pte = page_table_[i];
        if (pte.valid && pte.loaded_at < least_recent) {
            least_recent = pte.loaded_at;
            victim = i;
        }
    }
    assert(victim != static_cast<std::size_t>(-1));
    return victim;
}