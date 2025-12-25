#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class VirtualMemoryManager {
public:
    enum class PageReplacementPolicy {
        FIFO,
        LRU
    };

    VirtualMemoryManager(std::size_t num_virtual_pages,
                         std::size_t num_physical_frames,
                         std::size_t page_size_bytes,
                         PageReplacementPolicy policy = PageReplacementPolicy::FIFO);

    std::uint64_t translate(std::uint64_t virtual_address);
    std::size_t page_faults() const;
    std::uint64_t timestamp_;

private:
    struct PageTableEntry {
        bool valid;
        std::size_t frame_number;
        std::uint64_t loaded_at;

        PageTableEntry()
            : valid(false), frame_number(0), loaded_at(0) {}
    };

    std::size_t page_size_;
    std::size_t offset_bits_;

    std::vector<PageTableEntry> page_table_;
    std::vector<bool> frame_free_;
    std::size_t page_faults_;
    PageReplacementPolicy replacement_policy_;

    std::size_t decode_vpn(std::uint64_t virtual_address) const;
    std::size_t decode_offset(std::uint64_t virtual_address) const;
    std::size_t allocate_frame();

    std::size_t find_fifo_victim_page() const;
    std::size_t find_lru_victim_page() const;
};
