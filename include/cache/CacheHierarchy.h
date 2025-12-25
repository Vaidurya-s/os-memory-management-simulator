// #ifndef CACHE_HIERARCHY_H
// #define CACHE_HIERARCHY_H

// #include <vector>
// #include <memory>
// #include <cstdint>

// class CacheLevel {
// public:
//     CacheLevel(size_t size, size_t blockSize, size_t associativity);
//     ~CacheLevel();

//     bool access(uint64_t address, bool isWrite);
//     void invalidate(uint64_t address);
//     void flush();
    
//     size_t getHits() const { return hits; }
//     size_t getMisses() const { return misses; }
//     double getHitRate() const;
    
// private:
//     size_t size;
//     size_t blockSize;
//     size_t associativity;
//     size_t numSets;
    
//     size_t hits;
//     size_t misses;
    
//     struct CacheLine {
//         bool valid;
//         bool dirty;
//         uint64_t tag;
//         uint64_t lruCounter;
//     };
    
//     std::vector<std::vector<CacheLine>> cache;
//     uint64_t globalCounter;
    
//     size_t getSetIndex(uint64_t address) const;
//     uint64_t getTag(uint64_t address) const;
//     int findLine(size_t setIndex, uint64_t tag) const;
//     int findLRULine(size_t setIndex) const;
// };

// class CacheHierarchy {
// public:
//     CacheHierarchy();
//     ~CacheHierarchy();

//     void addLevel(size_t size, size_t blockSize, size_t associativity);
//     bool access(uint64_t address, bool isWrite = false);
//     void invalidate(uint64_t address);
//     void flushAll();
    
//     void printStatistics() const;
//     void resetStatistics();
    
//     size_t getNumLevels() const { return levels.size(); }
    
// private:
//     std::vector<std::unique_ptr<CacheLevel>> levels;
// };

// #endif // CACHE_HIERARCHY_H


#pragma once

#include "cache/DirectMappedCache.h"
#include <cstddef>
#include <cstdint>

class CacheHierarchy {
public:
    CacheHierarchy(DirectMappedCache l1,
                   DirectMappedCache l2);

    bool access(std::uint64_t physical_address);

    std::size_t l1_hits() const;
    std::size_t l1_misses() const;

    std::size_t l2_hits() const;
    std::size_t l2_misses() const;

private:
    DirectMappedCache l1_;
    DirectMappedCache l2_;
};
