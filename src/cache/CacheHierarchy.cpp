// #include "cache/CacheHierarchy.h"
// #include <iostream>
// #include <iomanip>
// #include <cmath>

// // CacheLevel Implementation

// CacheLevel::CacheLevel(size_t size, size_t blockSize, size_t associativity)
//     : size(size), blockSize(blockSize), associativity(associativity),
//       hits(0), misses(0), globalCounter(0) {
    
//     numSets = size / (blockSize * associativity);
//     cache.resize(numSets, std::vector<CacheLine>(associativity));
    
//     // Initialize all cache lines
//     for (auto& set : cache) {
//         for (auto& line : set) {
//             line.valid = false;
//             line.dirty = false;
//             line.tag = 0;
//             line.lruCounter = 0;
//         }
//     }
// }

// CacheLevel::~CacheLevel() {
// }

// size_t CacheLevel::getSetIndex(uint64_t address) const {
//     uint64_t blockNumber = address / blockSize;
//     return blockNumber % numSets;
// }

// uint64_t CacheLevel::getTag(uint64_t address) const {
//     uint64_t blockNumber = address / blockSize;
//     return blockNumber / numSets;
// }

// int CacheLevel::findLine(size_t setIndex, uint64_t tag) const {
//     const auto& set = cache[setIndex];
//     for (size_t i = 0; i < associativity; ++i) {
//         if (set[i].valid && set[i].tag == tag) {
//             return static_cast<int>(i);
//         }
//     }
//     return -1;
// }

// int CacheLevel::findLRULine(size_t setIndex) const {
//     const auto& set = cache[setIndex];
    
//     // First, try to find an invalid line
//     for (size_t i = 0; i < associativity; ++i) {
//         if (!set[i].valid) {
//             return static_cast<int>(i);
//         }
//     }
    
//     // If all lines are valid, find the LRU line
//     int lruIndex = 0;
//     uint64_t minCounter = set[0].lruCounter;
    
//     for (size_t i = 1; i < associativity; ++i) {
//         if (set[i].lruCounter < minCounter) {
//             minCounter = set[i].lruCounter;
//             lruIndex = static_cast<int>(i);
//         }
//     }
    
//     return lruIndex;
// }

// bool CacheLevel::access(uint64_t address, bool isWrite) {
//     size_t setIndex = getSetIndex(address);
//     uint64_t tag = getTag(address);
    
//     int lineIndex = findLine(setIndex, tag);
    
//     if (lineIndex != -1) {
//         // Cache hit
//         hits++;
//         cache[setIndex][lineIndex].lruCounter = ++globalCounter;
//         if (isWrite) {
//             cache[setIndex][lineIndex].dirty = true;
//         }
//         return true;
//     } else {
//         // Cache miss
//         misses++;
        
//         // Find a line to replace
//         int replaceIndex = findLRULine(setIndex);
        
//         // If the line being replaced is dirty, it would be written back
//         if (cache[setIndex][replaceIndex].valid && 
//             cache[setIndex][replaceIndex].dirty) {
//             // Write-back would occur here
//         }
        
//         // Replace the line
//         cache[setIndex][replaceIndex].valid = true;
//         cache[setIndex][replaceIndex].tag = tag;
//         cache[setIndex][replaceIndex].dirty = isWrite;
//         cache[setIndex][replaceIndex].lruCounter = ++globalCounter;
        
//         return false;
//     }
// }

// void CacheLevel::invalidate(uint64_t address) {
//     size_t setIndex = getSetIndex(address);
//     uint64_t tag = getTag(address);
    
//     int lineIndex = findLine(setIndex, tag);
//     if (lineIndex != -1) {
//         cache[setIndex][lineIndex].valid = false;
//         cache[setIndex][lineIndex].dirty = false;
//     }
// }

// void CacheLevel::flush() {
//     for (auto& set : cache) {
//         for (auto& line : set) {
//             line.valid = false;
//             line.dirty = false;
//             line.tag = 0;
//             line.lruCounter = 0;
//         }
//     }
// }

// double CacheLevel::getHitRate() const {
//     size_t total = hits + misses;
//     if (total == 0) return 0.0;
//     return static_cast<double>(hits) / total * 100.0;
// }

// // CacheHierarchy Implementation

// CacheHierarchy::CacheHierarchy() {
// }

// CacheHierarchy::~CacheHierarchy() {
// }

// void CacheHierarchy::addLevel(size_t size, size_t blockSize, size_t associativity) {
//     levels.push_back(std::make_unique<CacheLevel>(size, blockSize, associativity));
// }

// bool CacheHierarchy::access(uint64_t address, bool isWrite) {
//     bool hit = false;
    
//     // Try to access each level in order
//     for (size_t i = 0; i < levels.size(); ++i) {
//         if (levels[i]->access(address, isWrite)) {
//             // Hit at this level
//             hit = true;
//             break;
//         }
//         // Miss at this level, continue to next level
//     }
    
//     // If we missed at all levels, the data would be fetched from main memory
//     // and populated back through the hierarchy
    
//     return hit;
// }

// void CacheHierarchy::invalidate(uint64_t address) {
//     for (auto& level : levels) {
//         level->invalidate(address);
//     }
// }

// void CacheHierarchy::flushAll() {
//     for (auto& level : levels) {
//         level->flush();
//     }
// }

// void CacheHierarchy::printStatistics() const {
//     std::cout << "\n=== Cache Hierarchy Statistics ===\n";
    
//     for (size_t i = 0; i < levels.size(); ++i) {
//         std::cout << "L" << (i + 1) << " Cache:\n";
//         std::cout << "  Hits:     " << levels[i]->getHits() << "\n";
//         std::cout << "  Misses:   " << levels[i]->getMisses() << "\n";
//         std::cout << "  Hit Rate: " << std::fixed << std::setprecision(2) 
//                   << levels[i]->getHitRate() << "%\n";
//         std::cout << "\n";
//     }
// }

// void CacheHierarchy::resetStatistics() {
//     for (auto& level : levels) {
//         level->flush();
//     }
// }


#include "cache/CacheHierarchy.h"

CacheHierarchy::CacheHierarchy(DirectMappedCache l1,
                               DirectMappedCache l2)
    : l1_(std::move(l1)),
      l2_(std::move(l2)) {}

bool CacheHierarchy::access(std::uint64_t physical_address) {
    // Try L1
    if (l1_.access(physical_address)) {
        return true;
    }

    // L1 miss → try L2
    if (l2_.access(physical_address)) {
        // L2 hit → fill L1 only
        l1_.fill(physical_address);
        return true;
    }

    // L2 miss → fetch from memory
    l2_.fill(physical_address);
    l1_.fill(physical_address);

    return false;
}


std::size_t CacheHierarchy::l1_hits() const {
    return l1_.hits();
}

std::size_t CacheHierarchy::l1_misses() const {
    return l1_.misses();
}

std::size_t CacheHierarchy::l2_hits() const {
    return l2_.hits();
}

std::size_t CacheHierarchy::l2_misses() const {
    return l2_.misses();
}
