# Test Artifacts for Memory Simulator

This directory contains test workloads, expected outputs, and validation criteria for the memory simulator components.

## Directory Structure

```
test_artifacts/
├── workloads/           # Input workloads simulating memory allocation patterns
├── logs/                # Cache access logs and virtual address access logs
├── expected_outputs/    # Expected outputs and correctness criteria
└── scripts/             # Test execution and validation scripts
```

## Test Components

### 1. Physical Memory Allocator Tests

#### Test: Initialization
**Expected Behavior:**
- Total memory = allocated size
- Used memory = 0
- Free memory = total memory
- Largest free block = total memory

**Workload:** None (initialization only)
**Validation:** Memory metrics must match expected values

#### Test: First-Fit Allocation
**Expected Behavior:**
- Sequential allocations placed contiguously
- Used memory increases by allocation size
- Free memory decreases by allocation size

**Example Workload:**
```
Memory Size: 1024 bytes
Allocation 1: 100 bytes → id=1, used=100, free=924
Allocation 2: 200 bytes → id=2, used=300, free=724
```

**Validation Criteria:**
- Each allocation returns unique ID ≥ 0
- used_memory = sum of all allocations
- free_memory = total - used_memory

#### Test: Free and Reallocation
**Expected Behavior:**
- Freeing a block returns memory to free pool
- Subsequent allocations can reuse freed space
- Memory coalescing may occur for adjacent free blocks

**Example Workload:**
```
1. Allocate 100 bytes (id1)
2. Allocate 200 bytes (id2)
3. Allocate 150 bytes (id3)
4. Free id2 (200-byte hole created)
5. Allocate 50 bytes → should fit in freed hole
```

**Expected Result:**
- After step 3: used=450, free=574
- After step 4: used=250, free=774
- After step 5: used=300, free=724

### 2. Buddy Allocator Tests

#### Test: Power-of-Two Rounding
**Expected Behavior:**
- All allocations rounded up to nearest power of 2
- 100 bytes → 128 bytes
- 200 bytes → 256 bytes

**Validation:** Allocated size ≥ requested size and is power of 2

#### Test: Buddy Splitting and Coalescing
**Expected Behavior:**
- Allocation splits larger blocks into buddies
- Freeing adjacent buddies triggers coalescing
- After freeing all blocks: largest_free_block = total_memory

**Example Workload:**
```
Total: 1024 bytes
1. Allocate 128 bytes → addr1
2. Allocate 128 bytes → addr2 (buddy of addr1)
3. Free addr1
4. Free addr2 → Should coalesce into 256-byte block
```

**Expected Result:**
- After step 4: free_memory = 1024, largest_free_block = 1024

### 3. Cache Tests

#### Test: Cache Hit and Miss
**Expected Behavior:**
- First access to address → MISS
- Second access to same address → HIT (if not evicted)
- Hit ratio = hits / (hits + misses)

**Example Access Pattern:**
```
Cache: 1024 bytes, 64-byte lines (16 sets)
Access 0x1000 → MISS (misses=1, hits=0)
Fill 0x1000
Access 0x1000 → HIT (misses=1, hits=1)
Hit ratio = 1/2 = 0.5
```

#### Test: Cache Replacement (Conflict)
**Expected Behavior:**
- Addresses mapping to same set cause evictions
- Direct-mapped: new access evicts previous entry

**Example Conflict:**
```
Cache: 1024 bytes, 64-byte lines
addr1 = 0x0000 → set 0
addr2 = 0x0400 → set 0 (conflicts with addr1)

Access addr1 → MISS, fill
Access addr1 → HIT
Access addr2 → MISS, fill (evicts addr1)
Access addr1 → MISS (was evicted)
```

**Set Calculation:**
```
set_index = (address / line_size) % num_sets
```

### 4. Virtual Memory Tests

#### Test: Page Fault Behavior
**Expected Behavior:**
- First access to virtual page → page fault
- Subsequent accesses to same page → no fault
- Different pages → individual faults

**Example:**
```
Page size: 4096 bytes
Virtual pages: 64
Physical frames: 16

Access 0x2000 → Page 2, fault count = 1
Access 0x2100 → Page 2, fault count = 1 (same page)
Access 0x3000 → Page 3, fault count = 2
```

**Page Number Calculation:**
```
page_number = virtual_address / page_size
offset = virtual_address % page_size
```

#### Test: FIFO Page Replacement
**Expected Behavior:**
- Pages loaded in FIFO order
- When frames full, oldest page evicted first

**Example Workload:**
```
4 physical frames, 8 virtual pages

Step 1: Load pages [0,1,2,3] → 4 faults
Step 2: Access page 4 → evicts page 0, faults = 5
Step 3: Access page 0 → evicts page 1, faults = 6
```

**Frame State:**
```
Initial: []
After loading 0-3: [0,1,2,3]
After loading 4: [1,2,3,4]
After loading 0: [2,3,4,0]
```

#### Test: Address Translation
**Expected Behavior:**
- Physical address preserves offset from virtual address
- Frame number may differ but offset identical

**Calculation:**
```
virtual_address = page_number × page_size + offset
physical_address = frame_number × page_size + offset
offset = address & (page_size - 1)
```

**Example:**
```
Virtual: 0x1234
Page size: 4096 (0x1000)
Page number: 1
Offset: 0x234

If page 1 maps to frame 5:
Physical: 0x5234
```

## Expected Output Format

All test outputs include:
1. **[DEBUG]** - Configuration and setup information
2. **[STEP N]** - Test step description
3. **[CALC]** - Mathematical calculations shown
4. **[EXPECTED]** - Expected result before verification
5. **[ACTUAL]** - Actual result from system
6. **[STATE]** - Current system state
7. **[RESULT]** - Pass/Fail status

## Validation Criteria

### Physical Memory
- ✓ Total memory remains constant
- ✓ used_memory + free_memory = total_memory
- ✓ Allocation IDs are unique and ≥ 0
- ✓ Fragmentation ratio: 0.0 ≤ ratio ≤ 1.0

### Buddy Allocator
- ✓ All allocations are power of 2
- ✓ Buddy addresses differ by block size
- ✓ Coalescing restores original block
- ✓ Address alignment: addr % size == 0

### Cache
- ✓ 0 ≤ hit_ratio ≤ 1.0
- ✓ Set index < num_sets
- ✓ Offset < line_size
- ✓ Conflict detection: (addr1 / line_size) % sets == (addr2 / line_size) % sets

### Virtual Memory
- ✓ Physical address offset == virtual address offset
- ✓ Page faults increase monotonically
- ✓ Frame number < total_frames
- ✓ Page number < total_pages

## Running Tests with Debug Output

To see detailed debug output:

```bash
# Windows
cd build
.\test_physical_memory.exe
.\test_buddy_allocator.exe
.\test_cache.exe
.\test_virtual_memory.exe
.\test_page_table.exe

# Or run all tests
.\test_runner.exe
```

The debug output will show:
- Expected values calculated from input parameters
- Actual values returned by the system
- Step-by-step execution trace
- Validation of each assertion

## Test Coverage

| Component | Tests | Coverage |
|-----------|-------|----------|
| Physical Memory | 11 | Allocation strategies, fragmentation, metrics |
| Buddy Allocator | 12 | Splitting, coalescing, power-of-2, stress tests |
| Cache | 12 | Hit/miss, replacement, associativity, patterns |
| Virtual Memory | 11 | Translation, faults, FIFO/LRU, thrashing |
| Page Table | 10 | Entry management, timestamps, dirty/valid bits |

## Performance Benchmarks

Expected performance characteristics:
- Physical memory allocation: O(n) for n free blocks
- Buddy allocation: O(log n) for n levels
- Cache access: O(1) for direct-mapped
- Page table lookup: O(1) for single-level table

## Correctness Invariants

### Memory Allocators
1. No overlapping allocations
2. All allocated blocks within total memory bounds
3. Free list contains only valid free blocks
4. Sum of used and free equals total

### Cache
1. Each set contains ≤ ways entries
2. Tag uniquely identifies cache line
3. Valid bit set only for filled lines
4. LRU counters form valid ordering

### Virtual Memory
1. Valid bit clear implies no frame mapping
2. Frame numbers unique across valid pages
3. Page faults tracked correctly
4. Offset preservation in translation
