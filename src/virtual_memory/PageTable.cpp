#include "virtual_memory/PageTable.h"
#include <stdexcept>

PageTable::PageTable(std::size_t num_pages)
    : entries_(num_pages) {}

PageTableEntry& PageTable::entry(std::size_t vpn) {
    if (vpn >= entries_.size()) {
        throw std::out_of_range("VPN out of range");
    }
    return entries_[vpn];
}

const PageTableEntry& PageTable::entry(std::size_t vpn) const {
    if (vpn >= entries_.size()) {
        throw std::out_of_range("VPN out of range");
    }
    return entries_[vpn];
}

std::size_t PageTable::size() const {
    return entries_.size();
}
