# Memory Simulator - Component Integration

## Overview

The Memory Simulator now fully integrates three major components:
1. **Memory Allocation** (First Fit, Best Fit, Worst Fit, Buddy System)
2. **Virtual Memory** with page table translation
3. **Cache Hierarchy** (L1 and L2 caches)

## Integration Flow

When accessing memory with all components enabled, the flow is:

```
Virtual Address → Page Table → Physical Address → Cache → Physical Memory
```

### Detailed Steps:

1. **Virtual Address Translation**
   - Application uses virtual addresses (e.g., 0x1000)
   - VirtualMemoryManager translates VA to Physical Address
   - Page faults occur if page not in memory
   - Uses FIFO page replacement policy

2. **Cache Lookup**
   - Physical address is used to check L1 cache (32KB)
   - On L1 miss, checks L2 cache (256KB)
   - On L2 miss, accesses physical memory
   - Cache uses direct-mapped organization with 64-byte lines

3. **Physical Memory Access**
   - IAllocator interface manages physical memory
   - Supports four allocation strategies:
     - First Fit: Allocate from first suitable block
     - Best Fit: Allocate from smallest suitable block
     - Worst Fit: Allocate from largest suitable block
     - Buddy System: Power-of-2 sized blocks

## Usage

### Starting the Simulator

```bash
cd build
./memsim.exe
```

### Configuration Options

1. **Choose Allocator** (1-4):
   - 1 = First Fit
   - 2 = Best Fit
   - 3 = Worst Fit
   - 4 = Buddy System

2. **Set Memory Size**: Total physical memory in bytes (e.g., 65536)

3. **Enable Cache**: Y/N - Enables L1(32KB) + L2(256KB) hierarchy

4. **Enable Virtual Memory**: Y/N - Enables page table with 4KB pages

### Commands

#### Memory Allocation
- `malloc <size>` - Allocate memory block
- `free <block_id>` - Free allocated block
- `dump` - Show memory layout
- `stats` - Show memory statistics

#### Memory Access & Integration
- `access <vaddr>` - Access virtual address (shows full translation flow)
- `cache_stats` - Show L1/L2 hit rates
- `vm_stats` - Show page faults

#### General
- `help` - Show all commands
- `exit` or `quit` - Exit simulator

## Example Session

```
$ ./memsim.exe
Select Memory Allocator:
  1. First Fit
  2. Best Fit
  3. Worst Fit
  4. Buddy System

Enter choice (1-4): 1
Enter memory size (bytes): 65536

--- Optional Features ---
Enable cache hierarchy? (Y/N): Y
Enable virtual memory? (Y/N): Y

Memory simulator ready!
Flow: Virtual Address → Page Table → Physical Address → Cache → Memory

> malloc 1024
Allocated block id=1
  [Initial memory access after allocation]
    1. Virtual Address: 0x00001000
    2. Page Table Lookup → Physical Address: 0x00000000
       (Page fault occurred - page loaded into memory)
    3. Cache Access: L1 MISS, L2 MISS → Memory Access

> access 0x1000
  [Manual memory access]
    1. Virtual Address: 0x00001000
    2. Page Table Lookup → Physical Address: 0x00000000
    3. Cache Access: L1 HIT

> cache_stats
--- Cache Statistics ---
L1 Cache:
  Hits:   1
  Misses: 1
  Hit Rate: 50.00%

L2 Cache:
  Hits:   0
  Misses: 1

> vm_stats
--- Virtual Memory Statistics ---
Page faults: 1
Total accesses: 2
```

## Technical Details

### Cache Configuration
- **L1 Cache**: 32KB, 64-byte lines, direct-mapped
- **L2 Cache**: 256KB, 64-byte lines, direct-mapped
- Inclusive hierarchy (L1 data is subset of L2)

### Virtual Memory Configuration
- **Page Size**: 4KB (4096 bytes)
- **Virtual Pages**: 4× physical frames (overprovisioning)
- **Physical Frames**: max(1, memory_size / 4096)
  - Ensures at least 1 frame even for small memory sizes (< 4KB)
- **Replacement Policy**: FIFO (First-In-First-Out)
- **Virtual Address Spacing**: 256 bytes between block accesses
  - Allows multiple blocks to fit within the virtual address space
  - Works correctly with small memory configurations

### Memory Allocators

#### First/Best/Worst Fit (Variable-size)
- Maintains free list
- Coalesces adjacent free blocks
- Returns arbitrary-size allocations

#### Buddy System (Fixed-size)
- Power-of-2 sized blocks (32 bytes to 4MB)
- Splits larger blocks when needed
- Merges buddies on free
- Dual API: `allocate_buddy()` for raw addresses, `allocate()` for interface

## Testing

Test file: `test_inputs/test_integration.txt`

```
1          # First Fit allocator
65536      # 64KB memory
Y          # Enable cache
Y          # Enable virtual memory
help       # Show commands
malloc 1024
malloc 512
malloc 2048
dump
stats
access 0x1000
access 0x2000
access 0x1000
cache_stats
vm_stats
free 1
dump
stats
exit
```

Run test:
```bash
cd build
Get-Content ..\test_inputs\test_integration.txt | .\memsim.exe
```

## Architecture Benefits

1. **Modularity**: Each component (allocator, cache, VM) is independent
2. **Abstraction**: IAllocator interface allows swapping allocators
3. **Realism**: Simulates real OS memory management flow
4. **Observability**: Each layer reports its behavior (page faults, cache hits)
5. **Educational**: Shows VM→PA→Cache→Memory hierarchy clearly

## Key Implementation Files

- `src/main.cpp` - CLI and integration logic
- `include/allocator/IAllocator.h` - Abstract allocator interface
- `include/allocator/PhysicalMemory.h` - Variable-size allocators
- `include/buddy/BuddyAllocator.h` - Buddy system
- `include/virtual_memory/VirtualMemoryManager.h` - Page tables
- `include/cache/CacheHierarchy.h` - L1/L2 cache system

## Performance Observations

- **L1 Hit**: ~1 cycle (simulated)
- **L2 Hit**: ~10 cycles (simulated)
- **Memory Access**: ~100 cycles (simulated)
- **Page Fault**: ~10,000 cycles (simulated)

Cache hit rate depends on access patterns:
- Sequential: High L1 hit rate
- Random: Lower hit rates
- Repeated access (like 0x1000 three times): 100% hits after first miss
