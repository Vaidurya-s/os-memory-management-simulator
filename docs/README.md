# Memory Management Simulator

A comprehensive, user-space simulator for operating system memory management concepts, built with C++17. This project provides a complete educational platform for understanding memory allocation, virtual memory, caching mechanisms, and page replacement policies.

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Core Subsystems](#core-subsystems)
- [Installation](#installation)
- [Project Structure](#project-structure)
- [Design Principles](#design-principles)
- [Dependencies](#dependencies)
- [Contributing](#contributing)

## 🎯 Overview

This simulator implements a behavioral model of key operating system memory management techniques without requiring kernel-level access. It serves as an educational tool and testing platform for understanding:

- Physical memory allocation strategies
- Buddy allocation systems
- Multi-level cache hierarchies
- Virtual memory management
- Page replacement algorithms

All operations work with simulated memory, treating addresses as offsets from zero, ensuring complete isolation from actual system memory.

## ✨ Features

### Physical Memory Management
- **Variable-size allocator** with multiple allocation strategies:
  - First Fit
  - Best Fit
  - Worst Fit
- **Explicit metadata tracking** for all memory blocks
- **Automatic coalescing** of adjacent free blocks
- **Comprehensive metrics**:
  - Total/used/free memory
  - Largest free block size
  - External fragmentation calculation

### Buddy Allocation System
- **Power-of-two based allocation** for efficient memory management
- **Recursive splitting** on allocation
- **XOR-based buddy computation** for optimal pair identification
- **Automatic coalescing** on deallocation
- **Internal fragmentation tracking**
- **Invariant validation**:
  - No overlapping blocks
  - No unmerged buddy pairs
  - Complete memory conservation

### Cache Simulation
- **Configurable cache parameters**:
  - Cache size
  - Line size
  - Associativity (direct-mapped and set-associative)
- **FIFO replacement policy** with timestamp tracking
- **Correct address decomposition**:
  - Tag extraction
  - Index calculation
  - Offset isolation
- **Performance metrics**:
  - Hit/miss counts
  - Hit ratio calculation
  - Conflict miss detection

### Multi-Level Cache Hierarchy
- **L1-L2 cache configuration**
- **Inclusive hierarchy** (L2 accessed only on L1 miss)
- **Automatic cache line filling** across levels
- **Per-level statistics tracking**

### Virtual Memory Management
- **Page table implementation** with detailed entry metadata
- **Virtual-to-physical address translation**
- **Page fault handling**
- **Multiple page replacement policies**:
  - **FIFO** (First-In-First-Out)
  - **LRU** (Least Recently Used)
- **Warm-up and steady-state analysis support**
- **Comprehensive fault tracking**

## 🏗️ Architecture

### Component Hierarchy

```
┌─────────────────────────────────────┐
│     Application Layer (main.cpp)    │
└──────────────┬──────────────────────┘
               │
    ┌──────────┴──────────┐
    │                     │
┌───▼─────────┐   ┌───────▼──────────┐
│   Virtual   │   │  Cache Hierarchy │
│   Memory    │   │   (L1 → L2)      │
│  Manager    │   └──────────────────┘
└─────┬───────┘
      │
┌─────▼───────────────────────────────┐
│    Physical Memory Layer            │
│  ┌────────────┐   ┌──────────────┐  │
│  │  Standard  │   │    Buddy     │  │
│  │ Allocator  │   │  Allocator   │  │
│  └────────────┘   └──────────────┘  │
└─────────────────────────────────────┘
```

### Data Flow

1. **Virtual Memory Access**:
   - Virtual address → VirtualMemoryManager
   - Page table lookup
   - Page fault handling (if needed)
   - Physical address generation

2. **Cache Access**:
   - Physical address → Cache Hierarchy
   - L1 lookup
   - L2 lookup (on L1 miss)
   - Cache line fill from memory (on L2 miss)

3. **Physical Allocation**:
   - Allocation request → PhysicalMemory or BuddyAllocator
   - Block search/splitting
   - Metadata update
   - Address return

### ⚠️ Critical Integration Requirement

**When combining Virtual Memory and Cache components, the order of operations MUST be:**

```
Virtual Address → Page Table → Physical Address → Cache → Memory
```

**Correct Implementation:**
```cpp
// 1. Translate virtual to physical address
std::uint64_t physical_addr = vmm.translate(virtual_addr);

// 2. THEN access cache with physical address
bool hit = cache.access(physical_addr);
```

**⛔ WRONG - Never do this:**
```cpp
// Don't access cache with virtual address!
cache.access(virtual_addr);  // ❌ INCORRECT
```

**Rationale**: Caches operate on physical addresses in this simulator. Address translation must complete before cache lookup can begin. This mirrors real hardware where the TLB and page table produce physical addresses that are then used for cache indexing.

## 🔧 Core Subsystems

### 1. Physical Memory Allocator (`allocator/`)

**Purpose**: Manages variable-size block allocation with fragmentation tracking.

**Key Files**:
- `PhysicalMemory.h/cpp`: Main allocator implementation

**Key Components**:
- `MemoryBlock`: Block metadata (start, size, free status, ID)
- Allocation strategies: First/Best/Worst fit
- Two-sided coalescing algorithm
- Fragmentation metrics

### 2. Buddy Allocator (`buddy/`)

**Purpose**: Implements power-of-two allocation for reduced external fragmentation.

**Key Files**:
- `BuddyAllocator.h/cpp`: Complete buddy system implementation

**Key Features**:
- Free list management by order
- Recursive splitting and coalescing
- XOR-based buddy address calculation
- Internal fragmentation tracking
- Correctness invariants

### 3. Cache Simulator (`cache/`)

**Purpose**: Models CPU cache behavior with configurable parameters.

**Key Files**:
- `DirectMappedCache.h/cpp`: Generic cache implementation
- `CacheHierarchy.h/cpp`: Multi-level cache coordination

**Key Features**:
- Address decomposition (tag/index/offset)
- Set-associative support
- FIFO replacement with timestamps
- Hit/miss tracking
- Cache line fill mechanism

### 4. Virtual Memory (`virtual_memory/`)

**Purpose**: Simulates virtual memory with page-based address translation.

**Key Files**:
- `VirtualMemoryManager.h/cpp`: Main VMM implementation
- `PageTable.h/cpp`: Page table structure
- `VirtualAddress.h/cpp`: Address decoding utilities

**Key Features**:
- Virtual page number (VPN) to frame mapping
- Page fault detection and handling
- FIFO and LRU replacement policies
- Timestamp-based recency tracking
- Frame allocation management

## 🚀 Installation

### Prerequisites

- **C++ Compiler**: C++17 compliant (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake**: Version 3.10 or higher
- **Build Tools**: Make (Unix/Linux) or MSBuild (Windows)

### Build Instructions

#### Unix/Linux/macOS

```bash
# Clone the repository
git clone <repository-url>
cd memory-simulator

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
make

# Run the simulator
./memsim
```

#### Windows (Visual Studio)

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake (for Visual Studio)
cmake .. -G "Visual Studio 16 2019"

# Build using MSBuild
cmake --build . --config Release

# Run the simulator
Release\memsim.exe
```

#### Windows (MinGW)

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "MinGW Makefiles"

# Build
mingw32-make

# Run
memsim.exe
```

### Compiler Warnings

The project is configured to treat all warnings as errors:
- **MSVC**: `/W4 /WX`
- **GCC/Clang**: `-Wall -Wextra -Wpedantic -Werror`

## 📁 Project Structure

```
memory-simulator/
├── CMakeLists.txt              # Build configuration
├── docs/                       # Documentation
│   ├── README.md              # This file
│   ├── usage.md               # Usage guide and examples
│   └── implemented.txt        # Implementation progress log
├── include/                    # Header files
│   ├── allocator/
│   │   └── PhysicalMemory.h
│   ├── buddy/
│   │   └── BuddyAllocator.h
│   ├── cache/
│   │   ├── CacheHierarchy.h
│   │   └── DirectMappedCache.h
│   └── virtual_memory/
│       ├── PageTable.h
│       ├── VirtualAddress.h
│       └── VirtualMemoryManager.h
├── src/                        # Implementation files
│   ├── main.cpp               # Application entry point
│   ├── allocator/
│   │   └── PhysicalMemory.cpp
│   ├── buddy/
│   │   └── BuddyAllocator.cpp
│   ├── cache/
│   │   ├── CacheHierarchy.cpp
│   │   └── DirectMappedCache.cpp
│   └── virtual_memory/
│       ├── PageTable.cpp
│       ├── VirtualAddress.cpp
│       └── VirtualMemoryManager.cpp
└── build/                      # Build artifacts (generated)
```

## 🎨 Design Principles

### Core Assumptions

1. **Simulated Memory**: All memory is simulated; no real pointers are returned to users
2. **Zero-based Addressing**: All addresses are offsets starting from 0
3. **External Metadata**: Allocation metadata stored outside simulated memory
4. **Deterministic Behavior**: Same inputs always produce same outputs
5. **Behavioral Model**: This is NOT an OS kernel, but a behavioral simulator

### Invariants

- **Memory Conservation**: Total memory size never changes
- **No Overlaps**: Memory blocks never overlap
- **Complete Coverage**: Sum of all block sizes equals total memory
- **Consistent State**: All data structures remain consistent across operations
- **Timestamp Monotonicity**: Timestamps always increase

### Development Approach

- **Incremental Implementation**: Features added step-by-step with validation
- **Correctness First**: Correctness prioritized over performance
- **Comprehensive Testing**: Each component validated independently
- **Clean Separation**: Clear boundaries between subsystems

## 📦 Dependencies

### Build Dependencies
- CMake 3.10+
- C++17 compliant compiler

### Runtime Dependencies
- C++ Standard Library (STL):
  - `<vector>`: Dynamic arrays
  - `<list>`: Linked lists for memory blocks
  - `<unordered_map>`: Hash tables for block tracking
  - `<cstddef>`, `<cstdint>`: Standard types
  - `<iostream>`: Output formatting
  - `<cmath>`: Mathematical operations (log2)

No external third-party libraries required.

## 🔍 Key Algorithms

### Physical Memory Allocation

**First Fit**: O(n) - First block that fits
```cpp
for each block:
    if free and size >= requested:
        allocate from this block
```

**Best Fit**: O(n) - Smallest block that fits
```cpp
best = null
for each block:
    if free and size >= requested:
        if best == null or size < best.size:
            best = block
```

**Worst Fit**: O(n) - Largest block that fits
```cpp
worst = null
for each block:
    if free and size >= requested:
        if worst == null or size > worst.size:
            worst = block
```

### Buddy Allocation

**Allocation**: O(log n)
```
1. Round size up to next power of 2
2. Find smallest free block >= size
3. Split recursively until target size reached
4. Return address
```

**Deallocation**: O(log n)
```
1. Compute buddy address using XOR
2. If buddy is free, merge and repeat
3. Otherwise, insert into free list
```

### Cache Access

**Address Decomposition**:
```
offset_bits = log2(line_size)
index_bits = log2(num_sets)

offset = address & ((1 << offset_bits) - 1)
index = (address >> offset_bits) & ((1 << index_bits) - 1)
tag = address >> (offset_bits + index_bits)
```

**FIFO Replacement**:
```
1. Check all ways in set for matching tag
2. If hit, return true
3. If miss, find oldest line (min timestamp)
4. Replace with new line, update timestamp
```

### Virtual Memory Translation

**Translation**:
```
vpn = virtual_address >> offset_bits
offset = virtual_address & ((1 << offset_bits) - 1)

if page_table[vpn].valid:
    frame = page_table[vpn].frame_number
else:
    handle_page_fault(vpn)
    frame = page_table[vpn].frame_number

physical_address = frame * page_size + offset
```

**FIFO Page Replacement**:
```
Find page with minimum loaded_at timestamp
Evict that page
Allocate its frame to new page
```

**LRU Page Replacement**:
```
Find page with minimum loaded_at timestamp (access time)
Evict that page
Allocate its frame to new page
```

## 🧪 Testing and Validation

### Comprehensive Test Suite

The project includes a complete test suite with **6 independent test executables** covering all components:

#### Test Files

1. **test_physical_memory** - Physical Memory Allocator Tests
   - First-fit, best-fit, worst-fit allocation strategies
   - Memory fragmentation tracking and calculation
   - Block coalescing and merging
   - Memory metrics (total, used, free, largest block)
   - Allocation failure scenarios
   - Multiple allocation patterns
   - Invalid free operations

2. **test_buddy_allocator** - Buddy Allocation System Tests
   - Power-of-two rounding and allocation
   - Buddy splitting and coalescing mechanisms
   - Free list management across all orders
   - Internal fragmentation metrics
   - Invariant checking (no overlaps, no free buddy pairs)
   - Stress testing with varied allocation sizes
   - Largest free block tracking

3. **test_cache** - Cache Simulator Tests
   - Cache hits and misses
   - Address decoding (tag, index, offset extraction)
   - Cache replacement (FIFO) behavior
   - Sequential and strided access patterns
   - Various associativity levels (1-way, 2-way, 4-way)
   - Conflict miss detection
   - Different cache sizes and line sizes
   - Hit ratio calculations

4. **test_virtual_memory** - Virtual Memory Manager Tests
   - Virtual to physical address translation
   - Page fault detection and handling
   - FIFO page replacement policy
   - LRU page replacement policy
   - Working set behavior
   - Thrashing scenarios
   - Page fault counting
   - Address translation correctness

5. **test_page_table** - Page Table Tests
   - Page table entry management
   - Valid, dirty, and referenced bits
   - Frame number assignment and tracking
   - Timestamp management
   - Boundary condition testing
   - Multiple entry manipulation

6. **test_virtual_address** - Virtual Address Decoder Tests
   - VPN (Virtual Page Number) extraction
   - Offset extraction
   - Various page sizes (512B - 16KB)
   - Boundary address testing
   - Address reconstruction verification
   - Sequential address patterns

### Building and Running Tests

#### Quick Start - Automated Testing

**Windows (Batch Script)**:
```bash
# Run the automated build and test script
build_and_test.bat
```

**Windows/Linux/macOS (PowerShell)**:
```bash
# Run the cross-platform PowerShell script
pwsh build_and_test.ps1
```

#### Manual Build and Test

```bash
# Create and enter build directory
mkdir -p build && cd build

# Configure and build
cmake ..
cmake --build .

# Run all tests using custom target
cmake --build . --target run_tests

# Or run individual tests
./test_physical_memory
./test_buddy_allocator
./test_cache
./test_virtual_memory
./test_page_table
./test_virtual_address
```

#### Building Without Tests

```bash
cmake .. -DBUILD_TESTS=OFF
cmake --build .
```

### Test Coverage

Each test suite provides comprehensive coverage:

- ✅ **Basic Functionality** - Core features work as expected
- ✅ **Edge Cases** - Boundary conditions and special scenarios
- ✅ **Error Handling** - Invalid inputs and failure modes
- ✅ **Performance Metrics** - Statistics and measurements
- ✅ **Stress Testing** - Heavy loads and complex patterns
- ✅ **Invariant Validation** - Data structure consistency

### Expected Test Output

All tests output clear pass/fail status:

```
=== Running PhysicalMemory Tests ===
Testing initialization... PASSED
Testing first-fit allocation... PASSED
Testing best-fit allocation... PASSED
Testing worst-fit allocation... PASSED
Testing free and reallocation... PASSED
Testing fragmentation calculation... PASSED
Testing memory metrics... PASSED
Testing allocation failure... PASSED
Testing multiple allocations... PASSED
Testing free with invalid ID... PASSED
Testing block coalescing... PASSED
=== All PhysicalMemory Tests Passed! ===
```

### Built-in Invariant Checks

**Buddy Allocator**:
- `check_no_free_buddy_pairs()`: Ensures no adjacent free buddies exist
- `check_no_overlaps()`: Validates no memory overlaps

**Physical Memory**:
- Automatic total memory conservation verification
- Block boundary validation
- Coalescing correctness

### Metrics

All subsystems provide detailed metrics for analysis:
- Memory utilization (used/free/total)
- Fragmentation (internal/external)
- Cache performance (hits/misses/ratio)
- Page fault rates

### Test Documentation

For detailed testing information, see [tests/README.md](../tests/README.md)

## 🤝 Contributing

This is an educational project. Contributions should focus on:
1. **Correctness**: All changes must maintain system invariants
2. **Clarity**: Code should be readable and well-documented
3. **Testing**: New features should include validation
4. **Documentation**: Update docs to reflect changes

## 📚 Learning Resources

### Recommended Reading Order

1. **Physical Memory Allocation**: Start with `PhysicalMemory` to understand basic allocation
2. **Buddy System**: Move to `BuddyAllocator` for advanced techniques
3. **Cache Simulation**: Study `DirectMappedCache` for cache behavior
4. **Cache Hierarchy**: Understand multi-level caches with `CacheHierarchy`
5. **Virtual Memory**: Complete the picture with `VirtualMemoryManager`

### Concepts Demonstrated

- **Memory Fragmentation**: Both external (standard allocator) and internal (buddy)
- **Allocation Strategies**: Trade-offs between first/best/worst fit
- **Power-of-Two Allocation**: Benefits and limitations
- **Cache Locality**: Impact on hit rates
- **Page Replacement**: FIFO vs LRU trade-offs
- **Address Translation**: Virtual to physical mapping

## 📄 License

This project is intended for educational purposes.

## 🙏 Acknowledgments

Built following systems programming best practices and OS textbook concepts, with emphasis on correctness and educational value.

---

**Version**: 1.0  
**Last Updated**: December 2025  
**Status**: Fully Implemented - All core subsystems operational
