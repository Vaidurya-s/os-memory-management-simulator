#pragma once

#include <cstddef>
#include <cstdint>

struct VirtualAddress {
    std::size_t vpn;
    std::size_t offset;
};

class VirtualAddressDecoder {
public:
    explicit VirtualAddressDecoder(std::size_t page_size_bytes);

    VirtualAddress decode(std::uint64_t virtual_address) const;

private:
    std::size_t page_size_;
    std::size_t offset_bits_;
};
