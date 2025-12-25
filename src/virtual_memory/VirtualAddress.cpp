#include "virtual_memory/VirtualAddress.h"

#include <cmath>
#include <stdexcept>

static bool is_power_of_two(std::size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

VirtualAddressDecoder::VirtualAddressDecoder(std::size_t page_size_bytes)
    : page_size_(page_size_bytes), offset_bits_(0)
{
    if (!is_power_of_two(page_size_)) {
        throw std::invalid_argument("Page size must be a power of two");
    }

    offset_bits_ = static_cast<std::size_t>(std::log2(page_size_));
}

VirtualAddress VirtualAddressDecoder::decode(std::uint64_t virtual_address) const {
    VirtualAddress va;

    std::uint64_t offset_mask = (1ULL << offset_bits_) - 1;

    va.offset = virtual_address & offset_mask;
    va.vpn = virtual_address >> offset_bits_;

    return va;
}
