#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

struct CacheAddress {
    std::uint64_t tag;
    std::size_t index;
    std::size_t offset;
};

struct CacheLine {
    bool valid;
    std::uint64_t tag;
    std::uint64_t inserted_at;
    CacheLine() : valid(false), tag(0), inserted_at(0) {}
};

class DirectMappedCache {
public:
    DirectMappedCache(std::size_t cache_size_bytes,
                      std::size_t line_size_bytes,
                      std::size_t associativity = 1);

    std::size_t num_sets() const;

    CacheAddress decode_address(std::uint64_t physical_address) const;
    bool access(std::uint64_t physical_address);
    void fill(std::uint64_t physical_address);

    std::size_t hits() const;
    std::size_t misses() const;
    double hit_ratio() const;

private:
    std::size_t cache_size_;
    std::size_t line_size_;
    std::size_t associativity_;
    std::size_t num_sets_;

    std::size_t offset_bits_;
    std::size_t index_bits_;

    std::size_t hits_;
    std::size_t misses_;
    std::uint64_t timestamp_;

    std::vector<std::vector<CacheLine>> sets_;
};

