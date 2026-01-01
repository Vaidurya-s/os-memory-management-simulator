# Design Document: OS Memory Management Simulator

**Project:** Operating System Memory Management Simulator  
**Repository:** Vaidurya-s/os-memory-management-simulator  
**Version:** 1.0  
**Date:** January 2026  
**Language:** C++17  

---

## Table of Contents

1. [Overview](#1-overview)
2. [Memory Layout and Assumptions](#2-memory-layout-and-assumptions)
3. [Allocation Strategy Implementations](#3-allocation-strategy-implementations)
4. [Buddy System Design](#4-buddy-system-design)
5. [Cache Hierarchy and Replacement Policy](#5-cache-hierarchy-and-replacement-policy)
6. [Virtual Memory Model](#6-virtual-memory-model)
7. [Address Translation Flow](#7-address-translation-flow)
8. [Limitations and Simplifications](#8-limitations-and-simplifications)
9. [System Architecture](#9-system-architecture)

---

## 1. Overview

### 1.1 Purpose

This simulator implements a **behavioral model** of key operating system memory management techniques without requiring kernel-level access. It serves as an educational tool and testing platform for understanding: 

- Physical memory allocation strategies (First/Best/Worst Fit)
- Buddy allocation systems
- Multi-level cache hierarchies (L1-L2)
- Virtual memory management
- Page replacement algorithms (FIFO, LRU)

### 1.2 Scope

**What This Is:**
- User-space educational simulator
- Behavioral model of OS memory management
- Complete implementation with testing infrastructure

**What This Is NOT:**
- Actual OS kernel implementation
- Production-grade memory allocator
- Hardware-level simulator
- Replacement for system allocators

---

## 2. Memory Layout and Assumptions

### 2.1 Core Memory Model

```
┌─────────────────────────────────────────────────────────────┐
│                 SIMULATED MEMORY SPACE                      │
│                                                             │
│  All memory is simulated as contiguous address space       │
│  Addresses are simple offsets from 0                       │
│  No actual pointers to real memory are returned            │
└─────────────────────────────────────────────────────────────┘

Address Space:  [0 ...  total_memory_size - 1]
```

### 2.2 Fundamental Assumptions

#### 2.2.1 Simulated Memory
```
┌──────────────────────────────────────────────┐
│  Simulated Memory (Conceptual)               │
│  ┌────────────────────────────────────────┐  │
│  │  Address 0                             │  │
│  │  Address 1                             │  │
│  │  ...                                   │  │
│  │  Address (total_size - 1)              │  │
│  └────────────────────────────────────────┘  │
│                                              │
│  Metadata (Actual C++ Objects)               │
│  ┌────────────────────────────────────────┐  │
│  │  std::list<MemoryBlock>                │  │
│  │  std::unordered_map<... >               │  │
│  │  std::vector<PageTableEntry>           │  │
│  └────────────────────────────────────────┘  │
└──────────────────────────────────────────────┘
```

**Key Points:**
1. **Zero-based Addressing**: All addresses start from 0
2. **External Metadata**: Allocation metadata stored outside simulated memory
3. **Offset-based**:  Addresses are offsets, not real pointers
4. **Isolated**: Complete isolation from actual system memory

#### 2.2.2 Memory Conservation Invariant

```
INVARIANT: total_memory_size = Σ(all_block_sizes)

For all time t:
  used_memory(t) + free_memory(t) = total_memory_size
```

#### 2.2.3 No Overlaps Invariant

```
INVARIANT: No two blocks overlap

For any blocks A and B:
  (A.end < B.start) OR (B.end < A.start)
  
Where: 
  A.start = starting address of block A
  A. end = A.start + A.size - 1
```

### 2.3 Physical Memory Layout (PhysicalMemory)

```
┌──────────────────────────────────────────────────────────┐
│                   PHYSICAL MEMORY                        │
├──────────────────────────────────────────────────────────┤
│ Block 1 (Free)    │ Block 2 (Allocated) │ Block 3 (Free)│
│ Start: 0          │ Start:  512          │ Start: 1024   │
│ Size: 512         │ Size: 512           │ Size: 2560    │
│ ID: -1            │ ID: 42              │ ID: -1        │
└──────────────────────────────────────────────────────────┘

Stored as:  std::list<MemoryBlock>

struct MemoryBlock {
    std::size_t start;   // Starting address
    std::size_t size;    // Block size in bytes
    bool free;           // Allocation status
    int id;              // Unique identifier (-1 if free)
};
```

### 2.4 Buddy System Layout

```
Memory Size: 4096 bytes (2^12)
Max Order: 12

Free Lists by Order: 
┌─────────────────────────────────────────────┐
│ Order 12 (4096 bytes): [0]                  │
│ Order 11 (2048 bytes): []                   │
│ Order 10 (1024 bytes): []                   │
│ ...                                          │
│ Order 0  (1 byte):     []                   │
└─────────────────────────────────────────────┘

After allocating 512 bytes (rounded to 512 = 2^9):
┌─────────────────────────────────────────────┐
│ Order 12: []                                │
│ Order 11: []                                │
│ Order 10: [2048]        ← buddy of [0-1023] │
│ Order 9:   [512]         ← buddy of [0-511]  │
│ ...                                          │
└─────────────────────────────────────────────┘

Allocated Blocks Map:
  Address 0 → Order 9 (512 bytes allocated)
```

### 2.5 Cache Memory Organization

```
Cache Configuration:
  Total Size: 256 bytes
  Line Size: 32 bytes
  Associativity: 2-way set-associative
  Number of Sets: 256 / (32 * 2) = 4 sets

Physical Layout: 
┌────────────────────────────────────────────────────┐
│ Set 0 │ Way 0 │ [valid][tag][timestamp][data]     │
│       │ Way 1 │ [valid][tag][timestamp][data]     │
├────────────────────────────────────────────────────┤
│ Set 1 │ Way 0 │ [valid][tag][timestamp][data]     │
│       │ Way 1 │ [valid][tag][timestamp][data]     │
├────────────────────────────────────────────────────┤
│ Set 2 │ Way 0 │ [valid][tag][timestamp][data]     │
│       │ Way 1 │ [valid][tag][timestamp][data]     │
├────────────────────────────────────────────────────┤
│ Set 3 │ Way 0 │ [valid][tag][timestamp][data]     │
│       │ Way 1 │ [valid][tag][timestamp][data]     │
└────────────────────────────────────────────────────┘

Stored as: vector<vector<CacheLine>>
            cache[set_index][way]
```

### 2.6 Virtual Memory Layout

```
Virtual Address Space: 8 pages × 4096 bytes = 32KB
Physical Memory: 4 frames × 4096 bytes = 16KB

Page Table: 
┌──────┬───────┬──────────┬───────────┐
│ VPN  │ Valid │  Frame   │ Timestamp │
├──────┼───────┼──────────┼───────────┤
│  0   │   1   │    2     │   105     │
│  1   │   1   │    0     │   107     │
│  2   │   0   │    -     │    -      │
│  3   │   1   │    1     │   103     │
│  4   │   1   │    3     │   108     │
│  5   │   0   │    -     │    -      │
│  6   │   0   │    -     │    -      │
│  7   │   0   │    -     │    -      │
└──────┴───────┴──────────┴───────────┘

Frame Table: 
┌───────┬─────────┐
│ Frame │  In Use │
├───────┼─────────┤
│   0   │   Yes   │
│   1   │   Yes   │
│   2   │   Yes   │
│   3   │   Yes   │
└───────┴─────────┘
```

---

## 3. Allocation Strategy Implementations

### 3.1 First Fit Algorithm

**Time Complexity:** O(n) where n = number of blocks

```
Algorithm:  First Fit Allocation
Input: size (requested size)
Output: block_id (or -1 on failure)

1. FOR each block in blocks_list:
2.     IF block.free AND block.size >= size:
3.         RETURN allocate_from_block(block, size)
4. RETURN -1  // Allocation failed
```

**Implementation Details:**

```cpp
int PhysicalMemory::allocate_first_fit(std::size_t size) {
    for (auto it = blocks_. begin(); it != blocks_.end(); ++it) {
        if (it->free && it->size >= size) {
            return allocate_from_block(it, size);
        }
    }
    return -1;  // No suitable block found
}
```

**Characteristics:**
- ✅ **Fast**:  Stops at first suitable block
- ⚠️ **Fragmentation**:  Moderate external fragmentation
- 📊 **Use Case**: General-purpose allocation

### 3.2 Best Fit Algorithm

**Time Complexity:** O(n)

```
Algorithm: Best Fit Allocation
Input: size
Output: block_id

1. best_block = NULL
2. FOR each block in blocks_list:
3.     IF block.free AND block.size >= size:
4.         IF best_block == NULL OR block.size < best_block.size:
5.             best_block = block
6. IF best_block != NULL:
7.     RETURN allocate_from_block(best_block, size)
8. RETURN -1
```

**Characteristics:**
- ⏱️ **Slower**: Examines all blocks
- ✅ **Lower Fragmentation**:  Minimizes leftover space
- 📊 **Use Case**: When minimizing fragmentation is critical

### 3.3 Worst Fit Algorithm

**Time Complexity:** O(n)

```
Algorithm: Worst Fit Allocation
Input: size
Output: block_id

1. worst_block = NULL
2. FOR each block in blocks_list:
3.     IF block.free AND block. size >= size:
4.         IF worst_block == NULL OR block.size > worst_block.size:
5.             worst_block = block
6. IF worst_block != NULL:
7.     RETURN allocate_from_block(worst_block, size)
8. RETURN -1
```

**Characteristics:**
- ⏱️ **Slower**: Examines all blocks
- ⚠️ **Higher Fragmentation**: Creates larger leftover blocks
- 📊 **Use Case**: When future large allocations expected

### 3.4 Block Allocation Process

```
Allocation from Block: 
┌────────────────────────────────────────────────┐
│ Before: [Free Block:  start=0, size=1024]      │
├────────────────────────────────────────────────┤
│ Request: 256 bytes                             │
├────────────────────────────────────────────────┤
│ After:  [Allocated:  start=0, size=256, id=42] │
│         [Free:  start=256, size=768]            │
└────────────────────────────────────────────────┘

If request size == block size:
  - Mark entire block as allocated
  - Assign ID

If request size < block size: 
  - Create new allocated block
  - Shrink free block
  - Insert allocated block before free block
```

### 3.5 Coalescing Algorithm

**Two-Sided Coalescing:**

```
Algorithm: Free Block with Coalescing
Input: block_id
Output: void

1. Find block with matching ID
2. Mark block as free (free = true, id = -1)
3. 
4. // Coalesce with previous block
5. IF previous block exists AND previous. free: 
6.     previous.size += current.size
7.     Remove current from list
8.     current = previous
9. 
10. // Coalesce with next block
11. IF next block exists AND next.free:
12.     current.size += next.size
13.     Remove next from list
```

**Diagram:**

```
Before Coalescing:
[Free: 512]  [Allocated: 256]  [Free: 256]

After freeing middle block:
[Free: 512]  [Free: 256]  [Free: 256]

After coalescing:
[Free: 1024]
```

---

## 4. Buddy System Design

### 4.1 Fundamental Concepts

**Power-of-Two Allocation:**
```
Requested Size → Allocated Size
  1-1      →        1
  2-2      →        2
  3-4      →        4
  5-8      →        8
  100-128  →      128
  500-512  →      512
```

**Buddy Relationship:**
```
For a block at address A with size 2^k:
  Buddy address = A XOR 2^k

Example (Total memory = 1024 = 2^10):
  Block A at address 0, size 512 (2^9):
    Buddy = 0 XOR 512 = 512
  
  Block B at address 512, size 512:
    Buddy = 512 XOR 512 = 0
```

### 4.2 Data Structures

```cpp
class BuddyAllocator {
private:
    std::size_t total_memory_;           // Must be power of 2
    std::size_t max_order_;              // log2(total_memory)
    
    // Free lists organized by order (block size = 2^order)
    // free_lists_[k] contains addresses of free blocks of size 2^k
    std::vector<std::list<std::size_t>> free_lists_;
    
    // Track allocated blocks:  address → order
    std::unordered_map<std::size_t, std::size_t> allocated_blocks_;
};
```

**Example Free Lists:**
```
Memory: 1024 bytes (2^10)
max_order = 10

free_lists_[0]:   []           // 1-byte blocks
free_lists_[1]:  []           // 2-byte blocks
free_lists_[2]:  []           // 4-byte blocks
... 
free_lists_[9]:  []           // 512-byte blocks
free_lists_[10]: [0]          // 1024-byte blocks (entire memory)
```

### 4.3 Allocation Algorithm

**Step-by-Step Process:**

```
Algorithm: Buddy Allocation
Input: size (requested size)
Output: address (or -1 on failure)

1. rounded_size = next_power_of_two(size)
2. target_order = log2(rounded_size)
3. 
4. // Find smallest available block
5. current_order = target_order
6. WHILE current_order <= max_order:
7.     IF free_lists_[current_order] is not empty:
8.         BREAK
9.     current_order++
10. 
11. IF current_order > max_order:
12.     RETURN -1  // No memory available
13. 
14. // Take block from free list
15. address = free_lists_[current_order].pop_front()
16. 
17. // Split down to target order
18. WHILE current_order > target_order:
19.     current_order--
20.     buddy_address = address + 2^current_order
21.     free_lists_[current_order]. push_front(buddy_address)
22. 
23. allocated_blocks_[address] = target_order
24. RETURN address
```

**Visualization:**

```
Request: 128 bytes (2^7)
Initial State:  free_lists_[10] = [0]  (1024 bytes)

Step 1: Remove 1024-byte block
  free_lists_[10] = []

Step 2: Split to 512 (order 9)
  Keep [0-511] for further splitting
  free_lists_[9] = [512]  ← buddy

Step 3: Split to 256 (order 8)
  Keep [0-255] for further splitting
  free_lists_[8] = [256]  ← buddy
  free_lists_[9] = [512]

Step 4: Split to 128 (order 7)
  Allocate [0-127]
  free_lists_[7] = [128]  ← buddy
  free_lists_[8] = [256]
  free_lists_[9] = [512]

Result: Address 0 allocated with 128 bytes
  allocated_blocks_[0] = 7
```

### 4.4 Deallocation Algorithm

```
Algorithm: Buddy Deallocation
Input: address
Output: void

1. IF address not in allocated_blocks_:
2.     RETURN  // Invalid free
3. 
4. order = allocated_blocks_[address]
5. allocated_blocks_.remove(address)
6. 
7. current_address = address
8. current_order = order
9. 
10. // Coalesce with buddy if possible
11. WHILE current_order < max_order:
12.     buddy_address = current_address XOR 2^current_order
13.     
14.     IF buddy_address NOT in free_lists_[current_order]: 
15.         BREAK  // Buddy not free
16.     
17.     // Merge with buddy
18.     free_lists_[current_order]. remove(buddy_address)
19.     current_address = min(current_address, buddy_address)
20.     current_order++
21. 
22. // Add coalesced block to free list
23. free_lists_[current_order].push_front(current_address)
```

**Coalescing Example:**

```
Free address 0 (size 128):

Initial: 
  free_lists_[7] = [128]      ← buddy of 0
  free_lists_[8] = [256]
  free_lists_[9] = [512]

Step 1: Check buddy at order 7
  Buddy of 0 = 0 XOR 128 = 128
  128 is in free_lists_[7] → Coalesce! 
  Remove 128 from free_lists_[7]
  Merged block: [0-255], order 8

Step 2: Check buddy at order 8
  Buddy of 0 = 0 XOR 256 = 256
  256 is in free_lists_[8] → Coalesce!
  Remove 256 from free_lists_[8]
  Merged block: [0-511], order 9

Step 3: Check buddy at order 9
  Buddy of 0 = 0 XOR 512 = 512
  512 is in free_lists_[9] → Coalesce!
  Remove 512 from free_lists_[9]
  Merged block: [0-1023], order 10

Step 4: Order 10 is max_order, stop
  Add 0 to free_lists_[10]

Final:  free_lists_[10] = [0]  (back to initial state)
```

### 4.5 Internal Fragmentation

```
Internal Fragmentation = (Allocated - Requested) / Allocated

Example:
  Request: 500 bytes
  Allocated: 512 bytes (2^9)
  Internal Fragmentation = (512 - 500) / 512 = 0.0234 = 2.34%
```

---

## 5. Cache Hierarchy and Replacement Policy

### 5.1 Cache Organization

**Address Decomposition:**

```
Physical Address (64-bit):
┌──────────────┬──────────────┬──────────────┐
│     Tag      │    Index     │    Offset    │
└──────────────┴──────────────┴──────────────┘
      |              |              |
      v              v              v
  Identifies    Selects       Byte within
  unique line   cache set     cache line

Bit Allocation:
  offset_bits = log2(line_size)
  index_bits = log2(num_sets)
  tag_bits = 64 - offset_bits - index_bits
```

**Example (256-byte cache, 32-byte lines, 2-way):**

```
Cache Size: 256 bytes
Line Size: 32 bytes
Associativity: 2-way
Number of Sets: 256 / (32 * 2) = 4 sets

Address 0x12345:
  Binary: ... 0001 0010 0011 0100 0101
  
  offset_bits = log2(32) = 5
  index_bits = log2(4) = 2
  tag_bits = 64 - 5 - 2 = 57
  
  Offset = 0x12345 & 0x1F = 0x05 (bits 0-4)
  Index = (0x12345 >> 5) & 0x03 = 0x1 (bits 5-6)
  Tag = 0x12345 >> 7 = 0x246 (bits 7+)
```

### 5.2 Cache Line Structure

```cpp
struct CacheLine {
    bool valid;                  // Is this line valid?
    std::uint64_t tag;           // Address tag
    std::uint64_t timestamp;     // For FIFO replacement
};

// Cache organized as: 
// cache_[set_index][way] = CacheLine
std::vector<std::vector<CacheLine>> cache_;
```

### 5.3 FIFO Replacement Policy

**Algorithm:**

```
Algorithm: Cache Access with FIFO
Input: physical_address
Output: hit (boolean)

1. Decode address into tag, index, offset
2. 
3. // Check all ways in the set
4. FOR way = 0 TO associativity - 1:
5.     IF cache_[index][way].valid AND cache_[index][way].tag == tag:
6.         hits_++
7.         RETURN true  // HIT
8. 
9. // MISS: Find victim using FIFO
10. misses_++
11. victim_way = find_oldest_line_in_set(index)
12. 
13. // Replace victim
14. cache_[index][victim_way].valid = true
15. cache_[index][victim_way].tag = tag
16. cache_[index][victim_way].timestamp = current_timestamp++
17. 
18. RETURN false  // MISS
```

**Finding Oldest Line (FIFO):**

```
Algorithm: Find FIFO Victim
Input: set_index
Output: way_index

1. min_timestamp = ∞
2. victim_way = 0
3. 
4. FOR way = 0 TO associativity - 1:
5.     IF cache_[set_index][way]. timestamp < min_timestamp:
6.         min_timestamp = cache_[set_index][way].timestamp
7.         victim_way = way
8. 
9. RETURN victim_way
```

### 5.4 Multi-Level Cache Hierarchy

**L1-L2 Inclusive Hierarchy:**

```
┌──────────────────────────────────────┐
│          L1 Cache                    │
│  Size: 256 bytes                     │
│  Lines: 32 bytes                     │
│  Associativity: 1-way (direct-mapped)│
└──────────────┬───────────────────────┘
               │
               v (on L1 miss)
┌──────────────────────────────────────┐
│          L2 Cache                    │
│  Size: 1024 bytes                    │
│  Lines: 32 bytes                     │
│  Associativity: 2-way                │
└──────────────┬───────────────────────┘
               │
               v (on L2 miss)
         Main Memory
```

**Access Flow:**

```
Algorithm:  Hierarchical Cache Access
Input: physical_address
Output: overall_hit

1. // Try L1 first
2. IF l1_. access(physical_address):
3.     RETURN true  // L1 hit
4. 
5. // L1 miss, try L2
6. IF l2_.access(physical_address):
7.     // L2 hit:  fill L1 (silent fill)
8.     l1_.fill(physical_address)
9.     RETURN true  // Overall hit
10. 
11. // Both missed: fill both levels
12. l2_.fill(physical_address)
13. l1_.fill(physical_address)
14. RETURN false  // Cache miss (memory access needed)
```

**Silent Fill:**
```
// Fill operation that doesn't update hit/miss statistics
void DirectMappedCache::fill(std:: uint64_t address) {
    // Decode address
    // Find victim using FIFO
    // Replace without incrementing miss counter
}
```

### 5.5 Cache Performance Metrics

```
Metrics Tracked:
  - hits_: Total cache hits
  - misses_: Total cache misses
  - hit_ratio: hits / (hits + misses)

For L1-L2 Hierarchy:
  - L1 hit rate = L1_hits / (L1_hits + L1_misses)
  - L2 hit rate = L2_hits / (L2_hits + L2_misses)
  - Overall hit rate = (L1_hits + L2_hits) / total_accesses
  
Average Access Time (AAT):
  AAT = L1_hit_rate × L1_time +
        L1_miss_rate × (L2_hit_rate × L2_time +
                        L2_miss_rate × Memory_time)
```

---

## 6. Virtual Memory Model

### 6.1 Page Table Structure

```cpp
struct PageTableEntry {
    bool valid;                   // Is page in physical memory?
    bool dirty;                   // Has page been modified?
    bool referenced;              // Has page been accessed?
    std::size_t frame_number;     // Physical frame number
    std::uint64_t loaded_at;      // Timestamp (for FIFO/LRU)
};

class VirtualMemoryManager {
private: 
    std::size_t page_size_;                  // Page size (must be power of 2)
    std::size_t offset_bits_;                // Bits for page offset
    std::vector<PageTableEntry> page_table_; // One entry per virtual page
    std::vector<bool> frame_free_;           // Free frame tracking
    std::size_t page_faults_;                // Fault counter
    std::uint64_t timestamp_;                // Global timestamp
    PageReplacementPolicy replacement_policy_; // FIFO or LRU
};
```

### 6.2 Address Space Organization

```
Virtual Address Space:
┌────────────────────────────────────────┐
│ Virtual Page 0  (4096 bytes)           │ ← VPN 0
├────────────────────────────────────────┤
│ Virtual Page 1  (4096 bytes)           │ ← VPN 1
├────────────────────────────────────────┤
│ Virtual Page 2  (4096 bytes)           │ ← VPN 2
├────────────────────────────────────────┤
│ ...                                     │
└────────────────────────────────────────┘

Physical Memory:
┌────────────────────────────────────────┐
│ Frame 0 (4096 bytes)                   │
├────────────────────────────────────────┤
│ Frame 1 (4096 bytes)                   │
├────────────────────────────────────────┤
│ Frame 2 (4096 bytes)                   │
├────────────────────────────────────────┤
│ Frame 3 (4096 bytes)                   │
└────────────────────────────────────────┘

Page size < Total virtual pages
  → Creates memory pressure
  → Triggers page replacement
```

### 6.3 Virtual Address Structure

```
Virtual Address (64-bit):
┌───────────────────────┬──────────────────┐
│   Virtual Page Number │   Page Offset    │
│         (VPN)         │                  │
└───────────────────────┴──────────────────┘
    Higher bits            offset_bits

offset_bits = log2(page_size)
VPN bits = 64 - offset_bits

Example (4096-byte pages):
  offset_bits = 12
  VPN = address >> 12
  Offset = address & 0xFFF
```

### 6.4 Page Replacement Policies

#### 6.4.1 FIFO (First-In-First-Out)

```
Algorithm:  FIFO Page Replacement
Input:  void
Output: victim_vpn

1. oldest_timestamp = ∞
2. victim_vpn = 0
3. 
4. FOR vpn = 0 TO num_virtual_pages - 1:
5.     IF page_table_[vpn].valid:
6.         IF page_table_[vpn].loaded_at < oldest_timestamp: 
7.             oldest_timestamp = page_table_[vpn].loaded_at
8.             victim_vpn = vpn
9. 
10. RETURN victim_vpn
```

**Characteristics:**
- ✅ Simple to implement
- ✅ Predictable behavior
- ⚠️ Suffers from Belady's Anomaly
- ⚠️ May evict frequently-used pages

**Example:**
```
Frames: 3
Access sequence:  1, 2, 3, 4, 1, 2, 5

Step  Access  Frames          Fault?   Victim
----  ------  -------------  ------  ------
1     1       [1, -, -]       Yes     -
2     2       [1, 2, -]       Yes     -
3     3       [1, 2, 3]       Yes     -
4     4       [4, 2, 3]       Yes     1 (oldest)
5     1       [4, 1, 3]       Yes     2
6     2       [4, 1, 2]       Yes     3
7     5       [5, 1, 2]       Yes     4

Total faults: 7
```

#### 6.4.2 LRU (Least Recently Used)

```
Algorithm: LRU Page Replacement
Input: void
Output: victim_vpn

1. oldest_access = ∞
2. victim_vpn = 0
3. 
4. FOR vpn = 0 TO num_virtual_pages - 1:
5.     IF page_table_[vpn].valid:
6.         IF page_table_[vpn].loaded_at < oldest_access:
7.             oldest_access = page_table_[vpn].loaded_at
8.             victim_vpn = vpn
9. 
10. RETURN victim_vpn

Note: loaded_at is updated on EVERY access for LRU
```

**Characteristics:**
- ✅ Better performance than FIFO
- ✅ Approximates optimal algorithm
- ⚠️ More complex tracking
- ⚠️ Higher overhead

**Example (same sequence):**
```
Frames: 3
Access sequence: 1, 2, 3, 4, 1, 2, 5

Step  Access  Frames          Fault?  Victim  Reason
----  ------  -------------  ------  ------  ----------------
1     1       [1, -, -]       Yes     -       
2     2       [1, 2, -]       Yes     -
3     3       [1, 2, 3]       Yes     -
4     4       [4, 2, 3]       Yes     1       (LRU:  page 1)
5     1       [4, 2, 1]       Yes     3       (LRU: page 3)
6     2       [4, 2, 1]       No      -       (hit)
7     5       [5, 2, 1]       Yes     4       (LRU: page 4)

Total faults: 6 (better than FIFO's 7)
```

### 6.5 Page Fault Handling

```
Algorithm: Page Fault Handler
Input: vpn (virtual page number)
Output: frame_number

1. page_faults_++
2. 
3. // Try to allocate free frame
4. FOR frame = 0 TO num_frames - 1:
5.     IF frame_free_[frame]:
6.         frame_free_[frame] = false
7.         GOTO step 14  // Frame allocated
8. 
9. // No free frames:  use replacement policy
10. IF policy == FIFO:
11.     victim_vpn = find_fifo_victim()
12. ELSE:   // LRU
13.     victim_vpn = find_lru_victim()
14. 
15. // Evict victim
16. frame = page_table_[victim_vpn].frame_number
17. page_table_[victim_vpn].valid = false
18. 
19. // Load new page
20. page_table_[vpn].frame_number = frame
21. page_table_[vpn].valid = true
22. page_table_[vpn].loaded_at = timestamp_++
23. 
24. RETURN frame
```

---

## 7. Address Translation Flow

### 7.1 Complete System Flow

```
┌──────────────────────────────────────────────────────────────┐
│                   Application Request                        │
│                  (Virtual Address)                           │
└─────────────────────────┬────────────────────────────────────┘
                          │
                          v
┌──────────────────────────────────────────────────────────────┐
│             STEP 1: Virtual Memory Translation               │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 1. Decode VPN and Offset                               │ │
│  │    VPN = virtual_address >> offset_bits                │ │
│  │    Offset = virtual_address & ((1 << offset_bits) - 1) │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                   │
│                          v                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 2. Page Table Lookup                                   │ │
│  │    pte = page_table_[VPN]                              │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                   │
│                          v                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 3. Check Valid Bit                                     │ │
│  │    IF pte.valid == false:                              │ │
│  │        Handle Page Fault                               │ │
│  │        (May trigger replacement)                       │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                   │
│                          v                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 4. Generate Physical Address                           │ │
│  │    physical_addr = frame_number * page_size + offset   │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────┬────────────────────────────────────┘
                          │
                          v
┌──────────────────────────────────────────────────────────────┐
│              STEP 2: Cache Hierarchy Access                  │
│                   (Physical Address)                         │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 1. Decode Physical Address                             │ │
│  │    Tag, Index, Offset = decode(physical_addr)          │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │                                   │
│                          v                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 2. L1 Cache Lookup                                     │ │
│  │    IF L1_hit:                                          │ │
│  │        RETURN data                                     │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │ (L1 miss)                         │
│                          v                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 3. L2 Cache Lookup                                     │ │
│  │    IF L2_hit:                                          │ │
│  │        Fill L1 (silent)                                │ │
│  │        RETURN data                                     │ │
│  └────────────────────────────────────────────────────────┘ │
│                          │ (L2 miss)                         │
│                          v                                   │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ 4. Memory Access                                       │ │
│  │    Fill L2                                             │ │
│  │    Fill L1                                             │ │
│  │    RETURN data                                         │ │
│  └────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────┘
```

### 7.2 Detailed Translation Example

```
Example Configuration:
  Virtual Pages: 8 (VPN 0-7)
  Physical Frames: 4 (Frame 0-3)
  Page Size: 4096 bytes (2^12)
  Virtual Address:  0x1234

Step 1: Decode Virtual Address
  VPN = 0x1234 >> 12 = 0x1
  Offset = 0x1234 & 0xFFF = 0x234

Step 2: Page Table Lookup
  page_table_[1]: 
    valid = true
    frame_number = 2
    loaded_at = 105

Step 3: Valid Check
  Page is valid → No page fault

Step 4: Physical Address Generation
  physical_addr = 2 * 4096 + 0x234
                = 8192 + 564
                = 8756 (0x2234)

Step 5: Cache Access (256-byte cache, 32-byte lines, 2-way)
  offset_bits = 5, index_bits = 2, tag_bits = 57
  
  Offset = 0x2234 & 0x1F = 0x14
  Index = (0x2234 >> 5) & 0x3 = 0x1
  Tag = 0x2234 >> 7 = 0x44
  
  Check cache_[1][0] and cache_[1][1] for tag 0x44
```

### 7.3 Page Fault Scenario

```
Example: Page Fault with FIFO Replacement

Initial State:
  Page Table: 
    VPN 0: valid=1, frame=0, timestamp=100
    VPN 1: valid=1, frame=1, timestamp=102
    VPN 2: valid=1, frame=2, timestamp=104
    VPN 3: valid=1, frame=3, timestamp=106
    VPN 4: valid=0
  
  All frames occupied! 

Access: Virtual Address 0x4000 (VPN 4)

Step 1: Page Table Lookup
  page_table_[4]. valid = false → PAGE FAULT!

Step 2: Find Victim (FIFO)
  Scan all valid pages for minimum timestamp
  VPN 0: timestamp = 100 ← VICTIM
  VPN 1: timestamp = 102
  VPN 2: timestamp = 104
  VPN 3: timestamp = 106

Step 3: Evict VPN 0
  frame_to_use = page_table_[0].frame_number = 0
  page_table_[0]. valid = false

Step 4: Load VPN 4
  page_table_[4]. frame_number = 0
  page_table_[4].valid = true
  page_table_[4].loaded_at = 108 (new timestamp)

Step 5: Continue Translation
  physical_addr = 0 * 4096 + 0 = 0x0000

Result: VPN 4 now in frame 0, VPN 0 evicted
```

---

## 8. Limitations and Simplifications

### 8.1 Memory Model Simplifications

#### 8.1.1 No Actual Memory
```
LIMITATION:  Simulated addresses only
  - No real pointers returned
  - Cannot dereference addresses
  - Memory contents not stored
  
IMPACT: Cannot simulate actual data reads/writes
JUSTIFICATION: Focus is on management algorithms, not data
```

#### 8.1.2 Zero-Based Addressing
```
LIMITATION: All addresses start from 0
  - Simplifies calculations
  - No address space randomization
  
IMPACT: Not realistic for security analysis
JUSTIFICATION: Easier to understand and debug
```

#### 8.1.3 External Metadata
```
LIMITATION: Metadata stored separately
  - Real allocators embed metadata
  - No metadata overhead in simulated space
  
IMPACT: Memory accounting differs from real systems
JUSTIFICATION:  Clearer separation of concerns
```

### 8.2 Allocation Algorithm Simplifications

#### 8.2.1 Sequential Block Search
```
LIMITATION: O(n) search for all strategies
  - Real systems use trees (O(log n))
  - No segregated free lists
  
IMPACT: Poor scalability for large memory
JUSTIFICATION: Simpler implementation, educational focus
```

#### 8.2.2 No Memory Compaction
```
LIMITATION: Fragmentation cannot be reversed
  - No block moving
  - No defragmentation
  
IMPACT:  External fragmentation accumulates
JUSTIFICATION: Complexity vs. educational value
```

### 8.3 Buddy System Simplifications

#### 8.3.1 Power-of-Two Requirement
```
LIMITATION: Total memory must be 2^n
  - Cannot use arbitrary sizes
  - Internal fragmentation unavoidable
  
IMPACT:  Wastes memory for non-power-of-two requests
EXAMPLE: Request 100 bytes → allocated 128 bytes (28% waste)
```

#### 8.3.2 Simple Free List
```
LIMITATION: Linked list per order
  - Real systems use bitmaps
  - No fast buddy finding
  
IMPACT: O(n) buddy search in pathological cases
JUSTIFICATION:  Simpler to understand
```

### 8.4 Cache Simplifications

#### 8.4.1 FIFO Only
```
LIMITATION: Only FIFO replacement
  - No LRU, LFU, random, etc.
  - Simpler but less realistic
  
IMPACT:  Cannot compare replacement policies
JUSTIFICATION:  FIFO is easiest to implement correctly
```

#### 8.4.2 No Write Policies
```
LIMITATION: No write-back or write-through
  - No dirty bits used
  - No write buffer
  
IMPACT: Cannot simulate write performance
JUSTIFICATION: Read-only focus simplifies design
```

#### 8.4.3 Simplified Coherence
```
LIMITATION: No multi-core cache coherence
  - Single cache hierarchy
  - No MESI/MOESI protocol
  
IMPACT: Cannot study parallel systems
JUSTIFICATION: Out of scope for basic simulator
```

### 8.5 Virtual Memory Simplifications

#### 8.5.1 Single-Level Page Table
```
LIMITATION: Flat page table
  - Real systems use multi-level (2-4 levels)
  - No inverted page tables
  
IMPACT: High memory overhead for large address spaces
EXAMPLE: 32-bit address with 4KB pages = 2^20 entries
```

#### 8.5.2 No TLB
```
LIMITATION: No Translation Lookaside Buffer
  - Every access requires page table lookup
  - No TLB hit/miss tracking
  
IMPACT:  Cannot measure TLB performance
JUSTIFICATION:  Adds significant complexity
```

#### 8.5.3 Simplified Page Fault
```
LIMITATION: Instant page load
  - No disk I/O simulation
  - No page fault latency
  - No I/O queuing
  
IMPACT: Cannot measure realistic page fault cost
JUSTIFICATION: Focus on replacement algorithms
```

#### 8.5.4 No Page Sharing
```
LIMITATION: No shared pages
  - One-to-one VPN to frame mapping
  - No copy-on-write
  
IMPACT: Cannot simulate process forking
JUSTIFICATION: Simplified memory model
```

### 8.6 Integration Limitations

#### 8.6.1 Static Configuration
```
LIMITATION: No runtime reconfiguration
  - Cache sizes fixed at creation
  - Page size cannot change
  - Frame count fixed
  
IMPACT: Cannot simulate adaptive systems
JUSTIFICATION: Simpler implementation
```

#### 8.6.2 No Concurrency
```
LIMITATION: Single-threaded
  - No race conditions
  - No locks needed
  - Deterministic behavior
  
IMPACT:  Cannot study synchronization
JUSTIFICATION: Educational focus on algorithms
```

#### 8.6.3 No Real Timing
```
LIMITATION: Logical time only
  - No wall-clock time
  - No performance measurement
  
IMPACT: Cannot measure actual speed
JUSTIFICATION: Behavioral simulation only
```

### 8.7 Comparison with Real Systems

| Feature | Simulator | Real OS |
|---------|-----------|---------|
| Memory Storage | Simulated | Physical RAM |
| Metadata | External | Embedded |
| Page Table | Single-level | Multi-level |
| TLB | None | Hardware TLB |
| Cache Coherence | None | MESI/MOESI |
| Write Policy | N/A | Write-back |
| Page Faults | Instant | Disk I/O |
| Concurrency | None | Multi-core |
| Address Space | 64-bit logical | 64-bit virtual |
| Performance | Deterministic | Variable |

---

## 9. System Architecture

### 9.1 Component Hierarchy

```
┌─────────────────────────────────────────────────────────┐
│              Application Layer                          │
│                  (main.cpp)                             │
│  - Configures components                                │
│  - Runs workload simulations                            │
│  - Collects statistics                                  │
└────────────────────┬────────────────────────────────────┘
                     │
         ┌───────────┴──────────┐
         │                      │
         v                      v
┌─────────────────┐    ┌──────────────────┐
│ Virtual Memory  │    │ Cache Hierarchy  │
│    Manager      │    │   (L1 → L2)      │
│                 │    │                  │
│ - Page table    │    │ - L1 cache       │
│ - Translation   │    │ - L2 cache       │
│ - FIFO/LRU      │    │ - FIFO policy    │
└────────┬────────┘    └──────────────────┘
         │
         │ (generates physical addresses)
         │
         v
┌─────────────────────────────────────────────────────────┐
│            Physical Memory Layer                        │
│                                                         │
│  ┌───────────────────┐      ┌──────────────────────┐   │
│  │ PhysicalMemory    │      │  BuddyAllocator      │   │
│  │                   │      │                      │   │
│  │ - First/Best/     │      │ - Power-of-two       │   │
│  │   Worst fit       │      │ - Free lists         │   │
│  │ - Coalescing      │      │ - XOR buddy calc     │   │
│  │ - Fragmentation   │      │ - Coalescing         │   │
│  └───────────────────┘      └──────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

### 9.2 Data Flow Diagram

```
User Request
     │
     v
┌──────────────────────────────────────────┐
│ 1. Virtual Address Provided              │
│    Example: 0x1234                       │
└──────────────┬───────────────────────────┘
               │
               v
┌──────────────────────────────────────────┐
│ 2. Virtual Memory Manager                │
│    - Decode VPN and Offset               │
│    - Lookup page table                   │
│    - Handle page fault if needed         │
│    - Return physical address             │
│    Example: 0x2234                       │
└──────────────┬───────────────────────────┘
               │
               v
┌──────────────────────────────────────────┐
│ 3. Cache Hierarchy                       │
│    a. Try L1 cache                       │
│       - Decode tag/index/offset          │
│       - Check for hit                    │
│    b. On L1 miss, try L2                 │
│    c. On L2 miss, access memory          │
│       - Fill L2 and L1                   │
└──────────────┬───────────────────────────┘
               │
               v
┌──────────────────────────────────────────┐
│ 4. Statistics Collection                 │
│    - Page faults                         │
│    - L1 hits/misses                      │
│    - L2 hits/misses                      │
│    - Hit ratios                          │
└──────────────────────────────────────────┘
```

### 9.3 Module Dependencies

```cpp
// Include hierarchy showing dependencies

main.cpp
  ├── virtual_memory/VirtualMemoryManager.h
  │     ├── virtual_memory/PageTable.h
  │     └── virtual_memory/VirtualAddress.h
  │
  ├── cache/CacheHierarchy.h
  │     └── cache/DirectMappedCache.h
  │
  ├── allocator/PhysicalMemory.h
  │
  └── buddy/BuddyAllocator.h

// No circular dependencies
// Clean separation of concerns
```

### 9.4 File Organization

```
os-memory-management-simulator/
├── include/                    # Header files
│   ├── allocator/
│   │   └── PhysicalMemory.h   # Variable-size allocator
│   ├── buddy/
│   │   └── BuddyAllocator. h   # Buddy system
│   ├── cache/
│   │   ├── DirectMappedCache. h # Single cache level
│   │   └── CacheHierarchy.h    # L1-L2 coordinator
│   └── virtual_memory/
│       ├── VirtualMemoryManager.h # VMM main logic
│       ├── PageTable.h            # Page table structure
│       └── VirtualAddress.h       # Address decoder
│
├── src/                        # Implementation files
│   ├── allocator/
│   │   └── PhysicalMemory.cpp
│   ├── buddy/
│   │   └── BuddyAllocator.cpp
│   ├── cache/
│   │   ├── DirectMappedCache.cpp
│   │   └── CacheHierarchy.cpp
│   ├── virtual_memory/
│   │   ├── VirtualMemoryManager.cpp
│   │   ├── PageTable. cpp
│   │   └── VirtualAddress.cpp
│   └── main.cpp                # Entry point
│
├── tests/                      # Test suite
│   ├── test_physical_memory.cpp
│   ├── test_buddy_allocator.cpp
│   ├── test_cache. cpp
│   ├── test_virtual_memory. cpp
│   ├── test_page_table. cpp
│   └── test_virtual_address.cpp
│
├── docs/                       # Documentation
│   ├── README.md
│   ├── USAGE.md
│   └── implemented. txt
│
└── CMakeLists.txt              # Build configuration
```

### 9.5 Build System

```cmake
# Compilation targets

Executable:  memsim
  Sources: 
    - src/main.cpp
    - src/allocator/PhysicalMemory.cpp
    - src/buddy/BuddyAllocator.cpp
    - src/cache/*. cpp
    - src/virtual_memory/*.cpp

Test Executables:
  - test_physical_memory
  - test_buddy_allocator
  - test_cache
  - test_virtual_memory
  - test_page_table
  - test_virtual_address

Compiler:  C++17 compliant
  - GCC 7+
  - Clang 5+
  - MSVC 2017+

Warnings:  Strict
  - -Wall -Wextra -Wpedantic
  - Treat warnings as errors
```

### 9.6 Testing Strategy

```
Test Coverage:
├── Unit Tests (67+ test cases)
│   ├── PhysicalMemory (11 tests)
│   ├── BuddyAllocator (12 tests)
│   ├── DirectMappedCache (12 tests)
│   ├── VirtualMemoryManager (12 tests)
│   ├── PageTable (10 tests)
│   └── VirtualAddress (10 tests)
│
├── Invariant Checks
│   ├── Memory conservation
│   ├── No overlaps
│   ├── No free buddy pairs
│   └── Timestamp monotonicity
│
└── Stress Tests
    ├── Large allocations
    ├── Fragmentation scenarios
    ├── Thrashing detection
    └── Replacement policy comparison
```

---

## Appendix A: Key Algorithms Summary

### Physical Memory

| Algorithm | Time | Space | Fragmentation |
|-----------|------|-------|---------------|
| First Fit | O(n) | O(n) | Moderate |
| Best Fit | O(n) | O(n) | Lower |
| Worst Fit | O(n) | O(n) | Higher |
| Coalescing | O(1) | O(1) | Reduces |

### Buddy System

| Operation | Time | Space |
|-----------|------|-------|
| Allocate | O(log n) | O(n) |
| Free | O(log n) | O(n) |
| Coalesce | O(log n) | - |
| Find Buddy | O(1) | - |

### Cache

| Operation | Time | Space |
|-----------|------|-------|
| Access | O(associativity) | O(cache_size) |
| Replace | O(associativity) | - |
| Fill | O(associativity) | - |

### Virtual Memory

| Operation | Time | Space |
|-----------|------|-------|
| Translate | O(1) | O(num_pages) |
| Page Fault | O(num_pages) | - |
| FIFO Victim | O(num_pages) | - |
| LRU Victim | O(num_pages) | - |

---

## Appendix B: Configuration Examples

### Small System
```cpp
// Minimal configuration for testing
PhysicalMemory mem(1024);            // 1KB
BuddyAllocator buddy(1024);          // 1KB
DirectMappedCache l1(256, 32, 1);    // 256B, direct-mapped
DirectMappedCache l2(512, 32, 2);    // 512B, 2-way
VirtualMemoryManager vmm(4, 2, 1024, FIFO);  // 4 pages, 2 frames
```

### Medium System
```cpp
// Moderate configuration
PhysicalMemory mem(65536);           // 64KB
BuddyAllocator buddy(65536);         // 64KB
DirectMappedCache l1(1024, 64, 2);   // 1KB, 2-way
DirectMappedCache l2(4096, 64, 4);   // 4KB, 4-way
VirtualMemoryManager vmm(64, 16, 4096, LRU);  // 64 pages, 16 frames
```

### Large System
```cpp
// Larger configuration
PhysicalMemory mem(1048576);         // 1MB
BuddyAllocator buddy(1048576);       // 1MB
DirectMappedCache l1(8192, 64, 4);   // 8KB, 4-way
DirectMappedCache l2(32768, 64, 8);  // 32KB, 8-way
VirtualMemoryManager vmm(1024, 256, 4096, LRU);  // 1024 pages, 256 frames
```

---

## Document Control

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | January 2026 | Design Team | Initial release |

**End of Design Document**