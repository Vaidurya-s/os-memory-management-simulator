#pragma once


#include <cstddef>
#include <cstdint>
#include <vector>

struct PageTableEntry {
    bool valid;
    bool dirty;
    bool referenced;
    std::size_t frame_number;
    std::uint64_t loaded_at;

    PageTableEntry()
        : valid(false), frame_number(0), loaded_at(0) {}
};

class PageTable {
public:
    PageTable(std::size_t num_pages);

    PageTableEntry& entry(std::size_t vpn);
    const PageTableEntry& entry(std::size_t vpn) const;

    std::size_t size() const;

private:
    std::vector<PageTableEntry> entries_;
};
