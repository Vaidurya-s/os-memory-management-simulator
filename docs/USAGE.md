# Usage Guide - Memory Management Simulator

This guide provides comprehensive examples and usage patterns for the Memory Management Simulator, demonstrating how to use each subsystem and interpret the results.

## 📑 Table of Contents

- [Quick Start](#quick-start)
- [Physical Memory Allocator](#physical-memory-allocator)
- [Buddy Allocator](#buddy-allocator)
- [Cache Simulator](#cache-simulator)
- [Cache Hierarchy](#cache-hierarchy)
- [Virtual Memory Manager](#virtual-memory-manager)
- [Complete Examples](#complete-examples)
- [Performance Analysis](#performance-analysis)
- [Common Patterns](#common-patterns)
- [Troubleshooting](#troubleshooting)

## 🚀 Quick Start

### Basic Example

```cpp
#include "allocator/PhysicalMemory.h"
#include <iostream>

int main() {
    // Create a 1KB physical memory
    PhysicalMemory mem(1024);
    
    // Allocate using first-fit strategy
    int block1 = mem.allocate_first_fit(256);
    int block2 = mem.allocate_first_fit(128);
    
    // Display memory state
    mem.dump();
    
    // Check fragmentation
    std::cout << "External fragmentation: " 
              << mem.external_fragmentation() << std::endl;
    
    // Free blocks
    mem.free_block(block1);
    mem.free_block(block2);
    
    return 0;
}
```

## 🗂️ Physical Memory Allocator

### Creating an Allocator

```cpp
#include "allocator/PhysicalMemory.h"

// Create physical memory with specified size (in bytes)
PhysicalMemory mem(4096);  // 4KB memory
```

### Allocation Strategies

#### First Fit Strategy
Allocates from the first free block that is large enough.

```cpp
int block_id = mem.allocate_first_fit(512);
if (block_id == -1) {
    std::cout << "Allocation failed!" << std::endl;
} else {
    std::cout << "Allocated block ID: " << block_id << std::endl;
}
```

**Characteristics**:
- **Speed**: Fast (stops at first fit)
- **Fragmentation**: Moderate
- **Use Case**: General-purpose allocation

#### Best Fit Strategy
Finds the smallest free block that fits the request.

```cpp
int block_id = mem.allocate_best_fit(512);
```

**Characteristics**:
- **Speed**: Slower (examines all blocks)
- **Fragmentation**: Lower (minimizes leftover space)
- **Use Case**: When minimizing fragmentation is important

#### Worst Fit Strategy
Allocates from the largest free block.

```cpp
int block_id = mem.allocate_worst_fit(512);
```

**Characteristics**:
- **Speed**: Slower (examines all blocks)
- **Fragmentation**: Higher (creates larger leftover blocks)
- **Use Case**: When future large allocations are expected

### Deallocation

```cpp
// Free a specific block by ID
mem.free_block(block_id);

// Automatic coalescing happens:
// - Merges with previous block if free
// - Merges with next block if free
```

### Metrics and Analysis

```cpp
// Get memory statistics
std::size_t total = mem.total_memory();
std::size_t used = mem.used_memory();
std::size_t free = mem.free_memory();
std::size_t largest = mem.largest_free_block();

// Calculate external fragmentation
// Returns: 1 - (largest_free_block / total_free_memory)
double frag = mem.external_fragmentation();

std::cout << "Total: " << total << " bytes\n";
std::cout << "Used: " << used << " bytes\n";
std::cout << "Free: " << free << " bytes\n";
std::cout << "Largest free block: " << largest << " bytes\n";
std::cout << "External fragmentation: " << frag << "\n";
```

### Viewing Memory State

```cpp
// Print detailed memory dump
mem.dump();

// Output format:
// [start - end] FREE
// [start - end] USED (id=N)
```

### Complete Physical Memory Example

```cpp
#include "allocator/PhysicalMemory.h"
#include <iostream>

int main() {
    PhysicalMemory mem(1024);
    
    std::cout << "=== Initial State ===" << std::endl;
    mem.dump();
    
    // Allocate several blocks
    int a = mem.allocate_first_fit(200);
    int b = mem.allocate_first_fit(150);
    int c = mem.allocate_first_fit(300);
    
    std::cout << "\n=== After Allocation ===" << std::endl;
    mem.dump();
    std::cout << "External fragmentation: " 
              << mem.external_fragmentation() << std::endl;
    
    // Free middle block
    mem.free_block(b);
    
    std::cout << "\n=== After Freeing Block " << b << " ===" << std::endl;
    mem.dump();
    
    // Free adjacent blocks (triggers coalescing)
    mem.free_block(a);
    mem.free_block(c);
    
    std::cout << "\n=== After Freeing All ===" << std::endl;
    mem.dump();
    
    return 0;
}
```

## 🔀 Buddy Allocator

### Creating a Buddy Allocator

```cpp
#include "buddy/BuddyAllocator.h"

// Total memory MUST be a power of 2
BuddyAllocator buddy(4096);  // 4KB = 2^12 bytes
```

### Allocation

```cpp
// Allocate memory (automatically rounds up to power of 2)
std::size_t addr = buddy.allocate(500);

if (addr == static_cast<std::size_t>(-1)) {
    std::cout << "Allocation failed!" << std::endl;
} else {
    std::cout << "Allocated at address: " << addr << std::endl;
}

// Note: 500 bytes rounded up to 512 bytes (2^9)
```

### Deallocation

```cpp
// Free allocated block
buddy.free_block(addr);

// Automatic buddy coalescing happens:
// - Computes buddy address using XOR
// - Merges if buddy is also free
// - Recursively merges up the tree
```

### Metrics

```cpp
std::size_t total = buddy.total_memory();
std::size_t allocated = buddy.allocated_memory();
std::size_t free = buddy.free_memory();
std::size_t largest = buddy.largest_free_block();

// Internal fragmentation: wasted space within allocated blocks
double internal_frag = buddy.internal_fragmentation();

std::cout << "Total: " << total << " bytes\n";
std::cout << "Allocated: " << allocated << " bytes\n";
std::cout << "Free: " << free << " bytes\n";
std::cout << "Largest free: " << largest << " bytes\n";
std::cout << "Internal fragmentation: " << internal_frag << "\n";
```

### Invariant Checking

```cpp
// Verify no unmerged buddy pairs exist
bool no_buddies = buddy.check_no_free_buddy_pairs();
std::cout << "No free buddy pairs: " << (no_buddies ? "PASS" : "FAIL") << "\n";

// Verify no overlapping blocks
bool no_overlaps = buddy.check_no_overlaps();
std::cout << "No overlaps: " << (no_overlaps ? "PASS" : "FAIL") << "\n";
```

### Viewing Free Lists

```cpp
// Display all free lists by order
buddy.dump_free_lists();

// Output format:
// Order 0 (size 1): <addresses>
// Order 1 (size 2): <addresses>
// ...
// Order 12 (size 4096): <addresses>
```

### Complete Buddy Example

```cpp
#include "buddy/BuddyAllocator.h"
#include <iostream>

int main() {
    BuddyAllocator buddy(1024);  // 1KB
    
    std::cout << "=== Initial State ===" << std::endl;
    buddy.dump_free_lists();
    
    // Allocate blocks
    std::size_t a = buddy.allocate(100);  // Rounded to 128
    std::size_t b = buddy.allocate(200);  // Rounded to 256
    std::size_t c = buddy.allocate(50);   // Rounded to 64
    
    std::cout << "\n=== After Allocations ===" << std::endl;
    std::cout << "Block A at: " << a << std::endl;
    std::cout << "Block B at: " << b << std::endl;
    std::cout << "Block C at: " << c << std::endl;
    buddy.dump_free_lists();
    
    // Check metrics
    std::cout << "\nInternal fragmentation: " 
              << buddy.internal_fragmentation() << std::endl;
    
    // Free blocks
    buddy.free_block(a);
    buddy.free_block(c);
    
    std::cout << "\n=== After Freeing A and C ===" << std::endl;
    buddy.dump_free_lists();
    
    // Check invariants
    std::cout << "\nInvariant checks:" << std::endl;
    std::cout << "No buddy pairs: " 
              << (buddy.check_no_free_buddy_pairs() ? "PASS" : "FAIL") << std::endl;
    std::cout << "No overlaps: " 
              << (buddy.check_no_overlaps() ? "PASS" : "FAIL") << std::endl;
    
    return 0;
}
```

## 💾 Cache Simulator

### Creating a Cache

```cpp
#include "cache/DirectMappedCache.h"

// DirectMappedCache(cache_size, line_size, associativity)
DirectMappedCache cache(
    1024,   // 1KB total cache size
    64,     // 64-byte cache lines
    1       // Direct-mapped (1-way)
);

// Set-associative cache (4-way)
DirectMappedCache cache4way(
    4096,   // 4KB total cache size
    64,     // 64-byte cache lines
    4       // 4-way set-associative
);
```

### Cache Access

```cpp
// Access a physical address
std::uint64_t addr = 0x1000;
bool hit = cache.access(addr);

if (hit) {
    std::cout << "Cache hit!" << std::endl;
} else {
    std::cout << "Cache miss!" << std::endl;
}
```

### Address Decomposition

```cpp
#include <iostream>

// Decode address into tag, index, offset
CacheAddress decoded = cache.decode_address(0x12345);

std::cout << "Tag: " << decoded.tag << std::endl;
std::cout << "Index: " << decoded.index << std::endl;
std::cout << "Offset: " << decoded.offset << std::endl;
```

### Cache Statistics

```cpp
std::size_t hits = cache.hits();
std::size_t misses = cache.misses();
double hit_ratio = cache.hit_ratio();

std::cout << "Hits: " << hits << std::endl;
std::cout << "Misses: " << misses << std::endl;
std::cout << "Hit ratio: " << hit_ratio << std::endl;
```

### Complete Cache Example

```cpp
#include "cache/DirectMappedCache.h"
#include <iostream>

int main() {
    // Create 256-byte cache, 32-byte lines, 2-way set-associative
    DirectMappedCache cache(256, 32, 2);
    
    std::cout << "Cache configuration:" << std::endl;
    std::cout << "Number of sets: " << cache.num_sets() << std::endl;
    
    // Access pattern demonstrating conflict misses
    std::uint64_t addresses[] = {
        0x0000,  // First access - miss
        0x0020,  // Different line - miss
        0x0000,  // Same line - hit
        0x0100,  // Same set, different tag - miss (potential conflict)
        0x0000,  // Might be evicted - depends on associativity
    };
    
    for (std::uint64_t addr : addresses) {
        bool hit = cache.access(addr);
        std::cout << "Access 0x" << std::hex << addr 
                  << ": " << (hit ? "HIT" : "MISS") << std::endl;
    }
    
    std::cout << std::dec;  // Back to decimal
    std::cout << "\nFinal statistics:" << std::endl;
    std::cout << "Hits: " << cache.hits() << std::endl;
    std::cout << "Misses: " << cache.misses() << std::endl;
    std::cout << "Hit ratio: " << cache.hit_ratio() << std::endl;
    
    return 0;
}
```

## 🏢 Cache Hierarchy

### Creating Multi-Level Cache

```cpp
#include "cache/CacheHierarchy.h"
#include "cache/DirectMappedCache.h"

// Create L1 and L2 caches
DirectMappedCache l1(
    256,    // 256 bytes
    64,     // 64-byte lines
    1       // Direct-mapped
);

DirectMappedCache l2(
    1024,   // 1KB
    64,     // 64-byte lines
    2       // 2-way set-associative
);

// Create hierarchy
CacheHierarchy hierarchy(l1, l2);
```

### Accessing the Hierarchy

```cpp
// Access flows: L1 → L2 → Memory
std::uint64_t addr = 0x1000;
bool hit = hierarchy.access(addr);

// Returns true if hit in L1 or L2
// Returns false if miss in both (fetched from memory)
```

### Hierarchy Statistics

```cpp
// L1 statistics
std::cout << "L1 Hits: " << hierarchy.l1_hits() << std::endl;
std::cout << "L1 Misses: " << hierarchy.l1_misses() << std::endl;

// L2 statistics (only accessed on L1 miss)
std::cout << "L2 Hits: " << hierarchy.l2_hits() << std::endl;
std::cout << "L2 Misses: " << hierarchy.l2_misses() << std::endl;

// Calculate effective hit rate
double l1_hr = static_cast<double>(hierarchy.l1_hits()) / 
               (hierarchy.l1_hits() + hierarchy.l1_misses());
double l2_hr = static_cast<double>(hierarchy.l2_hits()) / 
               (hierarchy.l2_hits() + hierarchy.l2_misses());

std::cout << "L1 hit rate: " << l1_hr << std::endl;
std::cout << "L2 hit rate: " << l2_hr << std::endl;
```

### Complete Hierarchy Example

```cpp
#include "cache/CacheHierarchy.h"
#include "cache/DirectMappedCache.h"
#include <iostream>

int main() {
    DirectMappedCache l1(256, 64, 1);
    DirectMappedCache l2(1024, 64, 2);
    CacheHierarchy cache(l1, l2);
    
    std::cout << "=== Cache Hierarchy Test ===" << std::endl;
    
    // Simulated access pattern
    std::uint64_t accesses[] = {
        0x0000,  // L1 miss, L2 miss, fill both
        0x0040,  // L1 miss, L2 miss
        0x0000,  // L1 hit
        0x1000,  // L1 miss, L2 miss
        0x0000,  // Depends on L1 capacity
    };
    
    for (std::uint64_t addr : accesses) {
        bool hit = cache.access(addr);
        std::cout << "Access 0x" << std::hex << addr 
                  << ": " << (hit ? "HIT" : "MISS") << std::dec << std::endl;
    }
    
    std::cout << "\n=== Final Statistics ===" << std::endl;
    std::cout << "L1 - Hits: " << cache.l1_hits() 
              << ", Misses: " << cache.l1_misses() << std::endl;
    std::cout << "L2 - Hits: " << cache.l2_hits() 
              << ", Misses: " << cache.l2_misses() << std::endl;
    
    return 0;
}
```

## 🗺️ Virtual Memory Manager

### Creating a Virtual Memory Manager

```cpp
#include "virtual_memory/VirtualMemoryManager.h"

VirtualMemoryManager vmm(
    8,      // 8 virtual pages
    4,      // 4 physical frames
    4096,   // 4KB page size
    VirtualMemoryManager::PageReplacementPolicy::FIFO  // Replacement policy
);

// Or use LRU policy
VirtualMemoryManager vmm_lru(
    8, 4, 4096,
    VirtualMemoryManager::PageReplacementPolicy::LRU
);
```

### Address Translation

```cpp
// Translate virtual address to physical address
std::uint64_t virtual_addr = 0x1234;
std::uint64_t physical_addr = vmm.translate(virtual_addr);

std::cout << "Virtual: 0x" << std::hex << virtual_addr << std::endl;
std::cout << "Physical: 0x" << physical_addr << std::dec << std::endl;

// Page fault occurs automatically if page not in memory
```

### Page Fault Tracking

```cpp
// Get total page fault count
std::size_t faults = vmm.page_faults();
std::cout << "Total page faults: " << faults << std::endl;
```

### Page Replacement Policies

#### FIFO (First-In-First-Out)
```cpp
VirtualMemoryManager vmm_fifo(
    num_pages, num_frames, page_size,
    VirtualMemoryManager::PageReplacementPolicy::FIFO
);

// Evicts the page that was loaded earliest
// Simple but may evict frequently-used pages
```

#### LRU (Least Recently Used)
```cpp
VirtualMemoryManager vmm_lru(
    num_pages, num_frames, page_size,
    VirtualMemoryManager::PageReplacementPolicy::LRU
);

// Evicts the page that hasn't been used for the longest time
// Better performance but more complex tracking
```

### Complete Virtual Memory Example

```cpp
#include "virtual_memory/VirtualMemoryManager.h"
#include <iostream>
#include <vector>

int main() {
    const std::size_t page_size = 4096;
    const std::size_t num_pages = 8;
    const std::size_t num_frames = 3;
    
    VirtualMemoryManager vmm(
        num_pages, num_frames, page_size,
        VirtualMemoryManager::PageReplacementPolicy::FIFO
    );
    
    std::cout << "=== Virtual Memory Test ===" << std::endl;
    std::cout << "Virtual pages: " << num_pages << std::endl;
    std::cout << "Physical frames: " << num_frames << std::endl;
    std::cout << "Page size: " << page_size << " bytes" << std::endl;
    
    // Access sequence (as page numbers)
    std::vector<std::uint64_t> sequence = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3};
    
    std::cout << "\nAccess sequence: ";
    for (auto vpn : sequence) {
        std::cout << vpn << " ";
    }
    std::cout << std::endl << std::endl;
    
    // Translate each virtual page
    for (std::uint64_t vpn : sequence) {
        std::uint64_t vaddr = vpn * page_size;
        std::size_t before = vmm.page_faults();
        
        std::uint64_t paddr = vmm.translate(vaddr);
        
        std::size_t after = vmm.page_faults();
        bool fault = (after > before);
        
        std::cout << "Page " << vpn 
                  << " -> " << (fault ? "FAULT" : "HIT  ")
                  << " (Total faults: " << after << ")" << std::endl;
    }
    
    std::cout << "\nFinal page fault count: " 
              << vmm.page_faults() << std::endl;
    
    return 0;
}
```

## � Virtual Memory + Cache Integration

### Critical: Proper Order of Operations

**REQUIRED FLOW**: Virtual Address → Page Table → Physical Address → Cache → Memory

```cpp
#include "virtual_memory/VirtualMemoryManager.h"
#include "cache/CacheHierarchy.h"
#include "cache/DirectMappedCache.h"
#include <iostream>

int main() {
    // Create virtual memory manager
    VirtualMemoryManager vmm(
        4,      // 4 virtual pages
        2,      // 2 physical frames
        4096,   // 4KB page size
        VirtualMemoryManager::PageReplacementPolicy::LRU
    );
    
    // Create cache hierarchy
    DirectMappedCache l1(256, 64, 1);
    DirectMappedCache l2(1024, 64, 2);
    CacheHierarchy cache(l1, l2);
    
    std::cout << "=== Virtual Memory + Cache Integration ===" << std::endl;
    
    // Virtual addresses to access
    std::uint64_t virtual_addrs[] = {
        0x0000,  // Page 0
        0x1000,  // Page 1
        0x2000,  // Page 2 (causes page fault, evicts page 0)
        0x1000,  // Page 1 (hit in page table)
        0x0000   // Page 0 (page fault again)
    };
    
    for (std::uint64_t vaddr : virtual_addrs) {
        std::cout << "\nVirtual Address: 0x" << std::hex << vaddr << std::dec << std::endl;
        
        // STEP 1: Translate virtual to physical (page table lookup)
        std::uint64_t paddr = vmm.translate(vaddr);
        std::cout << "  → Physical Address: 0x" << std::hex << paddr << std::dec << std::endl;
        
        // STEP 2: Access cache with physical address
        bool cache_hit = cache.access(paddr);
        std::cout << "  → Cache: " << (cache_hit ? "HIT" : "MISS") << std::endl;
    }
    
    // Display statistics
    std::cout << "\n=== Final Statistics ===" << std::endl;
    std::cout << "Page Faults: " << vmm.page_faults() << std::endl;
    std::cout << "L1 Hits: " << cache.l1_hits() 
              << ", Misses: " << cache.l1_misses() << std::endl;
    std::cout << "L2 Hits: " << cache.l2_hits() 
              << ", Misses: " << cache.l2_misses() << std::endl;
    
    return 0;
}
```

### Why This Order Matters

1. **Virtual addresses are logical** - they don't correspond to actual memory locations
2. **Physical addresses are real** - they map to actual memory/cache locations
3. **Caches use physical addresses** for indexing and tagging
4. **Page faults happen first** - must be resolved before cache access

### Common Mistake to Avoid

```cpp
// ❌ WRONG - Accessing cache with virtual address
for (std::uint64_t vaddr : virtual_addrs) {
    cache.access(vaddr);  // INCORRECT!
}

// ✅ CORRECT - Translate first, then cache
for (std::uint64_t vaddr : virtual_addrs) {
    std::uint64_t paddr = vmm.translate(vaddr);  // Translate
    cache.access(paddr);                          // Then cache
}
```

## �📊 Complete Examples

### Example 1: Memory Allocation Comparison

```cpp
#include "allocator/PhysicalMemory.h"
#include <iostream>

void test_strategy(const char* name, 
                   int (PhysicalMemory::*strategy)(std::size_t)) {
    PhysicalMemory mem(1024);
    
    std::vector<int> blocks;
    blocks.push_back((mem.*strategy)(200));
    blocks.push_back((mem.*strategy)(100));
    blocks.push_back((mem.*strategy)(300));
    
    std::cout << "\n=== " << name << " ===" << std::endl;
    std::cout << "External fragmentation: " 
              << mem.external_fragmentation() << std::endl;
    
    mem.dump();
}

int main() {
    test_strategy("First Fit", &PhysicalMemory::allocate_first_fit);
    test_strategy("Best Fit", &PhysicalMemory::allocate_best_fit);
    test_strategy("Worst Fit", &PhysicalMemory::allocate_worst_fit);
    
    return 0;
}
```

### Example 2: Cache Conflict Analysis

```cpp
#include "cache/DirectMappedCache.h"
#include <iostream>

int main() {
    DirectMappedCache cache(256, 32, 1);  // 8 sets, direct-mapped
    
    std::cout << "=== Conflict Miss Demonstration ===" << std::endl;
    std::cout << "Cache: 256 bytes, 32-byte lines, direct-mapped" << std::endl;
    std::cout << "Sets: " << cache.num_sets() << std::endl << std::endl;
    
    // Addresses that map to same set (256 bytes apart)
    std::uint64_t addr1 = 0x0000;
    std::uint64_t addr2 = 0x0100;  // Same set as addr1
    std::uint64_t addr3 = 0x0200;  // Same set as addr1 and addr2
    
    cache.access(addr1);
    std::cout << "Access 0x" << std::hex << addr1 << " - MISS (compulsory)" << std::endl;
    
    cache.access(addr2);
    std::cout << "Access 0x" << addr2 << " - MISS (conflict)" << std::endl;
    
    cache.access(addr1);
    std::cout << "Access 0x" << addr1 << " - MISS (evicted by addr2)" << std::endl;
    
    std::cout << std::dec << "\nMisses: " << cache.misses() << std::endl;
    
    return 0;
}
```

### Example 3: Page Replacement Comparison

```cpp
#include "virtual_memory/VirtualMemoryManager.h"
#include <iostream>
#include <vector>

void test_policy(VirtualMemoryManager::PageReplacementPolicy policy,
                 const std::string& name,
                 const std::vector<std::uint64_t>& sequence) {
    VirtualMemoryManager vmm(8, 3, 4096, policy);
    
    for (std::uint64_t vpn : sequence) {
        vmm.translate(vpn * 4096);
    }
    
    std::cout << name << " page faults: " 
              << vmm.page_faults() << std::endl;
}

int main() {
    std::vector<std::uint64_t> sequence = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    
    std::cout << "=== Page Replacement Comparison ===" << std::endl;
    std::cout << "Sequence: ";
    for (auto vpn : sequence) std::cout << vpn << " ";
    std::cout << std::endl << std::endl;
    
    test_policy(VirtualMemoryManager::PageReplacementPolicy::FIFO, 
                "FIFO", sequence);
    test_policy(VirtualMemoryManager::PageReplacementPolicy::LRU,  
                "LRU ", sequence);
    
    return 0;
}
```

## 📈 Performance Analysis

### Measuring External Fragmentation

```cpp
PhysicalMemory mem(1024);

// Perform allocations
std::vector<int> blocks;
for (int i = 0; i < 10; i++) {
    blocks.push_back(mem.allocate_first_fit(rand() % 100 + 50));
}

// Free some blocks to create fragmentation
for (int i = 0; i < blocks.size(); i += 2) {
    mem.free_block(blocks[i]);
}

double frag = mem.external_fragmentation();
std::cout << "External fragmentation: " << (frag * 100) << "%" << std::endl;
```

### Measuring Cache Performance

```cpp
DirectMappedCache cache(1024, 64, 2);

// Simulate workload
for (int i = 0; i < 1000; i++) {
    cache.access(rand() % 10000);
}

double hr = cache.hit_ratio();
std::cout << "Hit ratio: " << (hr * 100) << "%" << std::endl;

// Analyze miss types by examining access patterns
```

### Comparing Allocation Strategies

```cpp
void benchmark_strategy(const char* name,
                       int (PhysicalMemory::*strategy)(std::size_t)) {
    PhysicalMemory mem(10240);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<int> blocks;
    for (int i = 0; i < 100; i++) {
        blocks.push_back((mem.*strategy)(rand() % 500 + 100));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << name << ":" << std::endl;
    std::cout << "  Time: " << duration.count() << " μs" << std::endl;
    std::cout << "  Fragmentation: " << mem.external_fragmentation() << std::endl;
}
```

## 🔍 Common Patterns

### Pattern 1: Warm-up Phase for Virtual Memory

```cpp
// Run multiple rounds to stabilize state
for (int warmup = 0; warmup < 5; warmup++) {
    for (auto vpn : access_pattern) {
        vmm.translate(vpn * page_size);
    }
}

// Reset fault counter (conceptually)
std::size_t baseline = vmm.page_faults();

// Measure steady-state performance
for (auto vpn : access_pattern) {
    vmm.translate(vpn * page_size);
}

std::size_t steady_faults = vmm.page_faults() - baseline;
```

### Pattern 2: Cache Warming

```cpp
// Pre-load cache with expected data
for (std::uint64_t addr = 0; addr < dataset_size; addr += line_size) {
    cache.access(addr);
}

// Now measure performance
cache.hits();  // Reset or note starting point
```

### Pattern 3: Memory Pool Simulation

```cpp
PhysicalMemory pool(65536);  // 64KB pool

struct Allocation {
    int id;
    std::size_t size;
};

std::vector<Allocation> active;

// Allocate
for (int i = 0; i < 100; i++) {
    std::size_t size = rand() % 1000 + 100;
    int id = pool.allocate_best_fit(size);
    if (id != -1) {
        active.push_back({id, size});
    }
}

// Randomly free some
while (!active.empty()) {
    int idx = rand() % active.size();
    pool.free_block(active[idx].id);
    active.erase(active.begin() + idx);
}
```

## 🔧 Troubleshooting

### Common Issues and Solutions

#### Issue: Buddy Allocator Throws Exception
```
Error: "BuddyAllocator requires total memory to be a power of two"
```

**Solution**: Ensure memory size is 2^n (e.g., 1024, 2048, 4096)

```cpp
// ❌ Wrong
BuddyAllocator buddy(1000);

// ✅ Correct
BuddyAllocator buddy(1024);
```

#### Issue: Cache Configuration Error
```
Error: "Cache size, line size, and associativity must be non-zero"
```

**Solution**: Verify all parameters are positive and valid

```cpp
// ❌ Wrong
DirectMappedCache cache(256, 32, 0);

// ✅ Correct
DirectMappedCache cache(256, 32, 1);
```

#### Issue: Allocation Returns -1
**Cause**: Not enough free memory

**Solution**: Check available memory before allocation

```cpp
std::size_t available = mem.largest_free_block();
if (available >= requested_size) {
    int id = mem.allocate_first_fit(requested_size);
} else {
    std::cout << "Insufficient memory" << std::endl;
}
```

#### Issue: Page Faults Higher Than Expected
**Cause**: Working set exceeds physical frames

**Solution**: Either increase frames or analyze access pattern

```cpp
// Increase physical frames
VirtualMemoryManager vmm(8, 6, 4096, policy);  // More frames

// Or analyze working set
std::set<std::uint64_t> unique_pages;
for (auto vpn : sequence) {
    unique_pages.insert(vpn);
}
std::cout << "Working set size: " << unique_pages.size() << std::endl;
```

### Debugging Tips

1. **Use dump() methods**: Visualize memory state
```cpp
mem.dump();
buddy.dump_free_lists();
```

2. **Check invariants**: Validate correctness
```cpp
assert(buddy.check_no_free_buddy_pairs());
assert(buddy.check_no_overlaps());
```

3. **Track metrics**: Monitor performance
```cpp
std::cout << "Free memory: " << mem.free_memory() << std::endl;
std::cout << "Fragmentation: " << mem.external_fragmentation() << std::endl;
```

4. **Validate inputs**: Check before operations
```cpp
if (size > mem.total_memory()) {
    std::cerr << "Request exceeds total memory!" << std::endl;
    return -1;
}
```

## 📝 Best Practices

1. **Always check allocation results**: Never assume success
2. **Free memory in LIFO order when possible**: Reduces fragmentation
3. **Use appropriate allocation strategy**: Match to workload characteristics
4. **Monitor fragmentation**: Track over time to detect issues
5. **Warm up caches and VMM**: For accurate steady-state measurements
6. **Validate with invariants**: Use built-in checks regularly
7. **Choose page size carefully**: Balance table size vs. fragmentation
8. **Match cache line size to access patterns**: Optimize for spatial locality

---

For more information, see the [README.md](README.md) for architecture details and design principles.