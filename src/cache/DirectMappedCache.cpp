#include "cache/DirectMappedCache.h"

#include <cmath>
#include <stdexcept>

static bool is_power_of_two(std::size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

DirectMappedCache::DirectMappedCache(std::size_t cache_size_bytes,
                                     std::size_t line_size_bytes,
                                     std::size_t associativity)
    : cache_size_(cache_size_bytes),
      line_size_(line_size_bytes),
      associativity_(associativity),
      num_sets_(0),
      offset_bits_(0),
      index_bits_(0),
      hits_(0),
      misses_(0),
      timestamp_(0)
{
    if (cache_size_ == 0 || line_size_ == 0 || associativity_ == 0) {
        throw std::invalid_argument("Cache size, line size, and associativity must be non-zero");
    }

    if (cache_size_ % (line_size_ * associativity_) != 0) {
        throw std::invalid_argument("Cache size must be divisible by line_size * associativity");
    }

    num_sets_ = cache_size_ / (line_size_ * associativity_);

    if (!is_power_of_two(line_size_) || !is_power_of_two(num_sets_)) {
        throw std::invalid_argument("Line size and number of sets must be powers of two");
    }

    offset_bits_ = static_cast<std::size_t>(std::log2(line_size_));
    index_bits_  = static_cast<std::size_t>(std::log2(num_sets_));

    sets_.resize(num_sets_, std::vector<CacheLine>(associativity_));
}


std::size_t DirectMappedCache::num_sets() const {
    return num_sets_;
}

CacheAddress DirectMappedCache::decode_address(std::uint64_t physical_address) const {
    CacheAddress addr;

    std::uint64_t offset_mask = (1ULL << offset_bits_) - 1;
    std::uint64_t index_mask  = (1ULL << index_bits_) - 1;

    addr.offset = physical_address & offset_mask;
    addr.index  = (physical_address >> offset_bits_) & index_mask;
    addr.tag    = physical_address >> (offset_bits_ + index_bits_);

    return addr;
}


bool DirectMappedCache::access(std::uint64_t physical_address) {
    CacheAddress addr = decode_address(physical_address);
    auto& set = sets_[addr.index];

    for (auto& line : set) {
        if (line.valid && line.tag == addr.tag) {
            ++hits_;
            return true;
        }
    }

    ++misses_;

    CacheLine* victim = nullptr;

    for (auto& line : set) {
        if (!line.valid) {
            victim = &line;
            break;
        }
    }

    if (!victim) {
        victim = &set[0];
        for (auto& line : set) {
            if (line.inserted_at < victim->inserted_at) {
                victim = &line;
            }
        }
    }

    victim->valid = true;
    victim->tag = addr.tag;
    victim->inserted_at = timestamp_++;

    return false;
}


std::size_t DirectMappedCache::hits() const {
    return hits_;
}

std::size_t DirectMappedCache::misses() const {
    return misses_;
}

double DirectMappedCache::hit_ratio() const {
    std::size_t total = hits_ + misses_;
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(hits_) / total;
}


void DirectMappedCache::fill(std::uint64_t physical_address) {
    CacheAddress addr = decode_address(physical_address);
    auto& set = sets_[addr.index];

    CacheLine* victim = nullptr;

    for (auto& line : set) {
        if (!line.valid) {
            victim = &line;
            break;
        }
    }

    if (!victim) {
        victim = &set[0];
        for (auto& line : set) {
            if (line.inserted_at < victim->inserted_at) {
                victim = &line;
            }
        }
    }

    victim->valid = true;
    victim->tag = addr.tag;
    victim->inserted_at = timestamp_++;
}
