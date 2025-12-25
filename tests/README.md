# Memory Simulator - Test Suite

This directory contains comprehensive unit tests for all components of the memory simulator.

## Test Files

- **test_physical_memory.cpp** - Tests for the PhysicalMemory allocator
  - First-fit, best-fit, and worst-fit allocation strategies
  - Memory fragmentation tracking
  - Block coalescing
  - Memory metrics and statistics

- **test_buddy_allocator.cpp** - Tests for the BuddyAllocator
  - Buddy system allocation and deallocation
  - Block splitting and coalescing
  - Power-of-two rounding
  - Internal fragmentation metrics
  - Invariant checking (no overlaps, no free buddy pairs)

- **test_cache.cpp** - Tests for the DirectMappedCache
  - Cache hits and misses
  - Address decoding (tag, index, offset)
  - Cache replacement policies
  - Sequential and strided access patterns
  - Various associativity levels
  - Conflict miss detection

- **test_virtual_memory.cpp** - Tests for the VirtualMemoryManager
  - Virtual to physical address translation
  - Page fault handling
  - FIFO and LRU page replacement policies
  - Working set behavior
  - Thrashing scenarios

- **test_page_table.cpp** - Tests for the PageTable
  - Page table entry management
  - Valid, dirty, and referenced bits
  - Frame number assignment
  - Timestamp tracking

- **test_virtual_address.cpp** - Tests for the VirtualAddressDecoder
  - Virtual address decomposition (VPN and offset)
  - Various page sizes (512B, 1KB, 2KB, 4KB, 8KB, 16KB)
  - Boundary condition testing
  - Address reconstruction verification

- **test_runner.cpp** - Main test runner application

## Building the Tests

### Using CMake (Recommended)

```bash
# From the project root directory
mkdir -p build
cd build
cmake ..
cmake --build .
```

This will build:
- `test_physical_memory.exe`
- `test_buddy_allocator.exe`
- `test_cache.exe`
- `test_virtual_memory.exe`
- `test_page_table.exe`
- `test_virtual_address.exe`
- `test_runner.exe`

### Running All Tests

```bash
# From the build directory
cmake --build . --target run_tests
```

Or manually run each test:

```bash
./test_physical_memory
./test_buddy_allocator
./test_cache
./test_virtual_memory
./test_page_table
./test_virtual_address
```

### Building Individual Tests

To build only specific tests:

```bash
cmake --build . --target test_physical_memory
cmake --build . --target test_buddy_allocator
cmake --build . --target test_cache
# etc.
```

### Disabling Tests

To build without tests:

```bash
cmake .. -DBUILD_TESTS=OFF
cmake --build .
```

## Test Coverage

Each test suite includes comprehensive coverage of:

1. **Basic Functionality** - Core features work as expected
2. **Edge Cases** - Boundary conditions and special cases
3. **Error Handling** - Invalid inputs and failure scenarios
4. **Performance Characteristics** - Metrics and statistics
5. **Stress Testing** - Heavy load and complex scenarios
6. **Invariant Checking** - Data structure consistency

## Expected Output

All tests should output:
- Test name being executed
- "PASSED" for each successful test
- Summary of total tests passed

Example:
```
=== Running PhysicalMemory Tests ===
Testing initialization... PASSED
Testing first-fit allocation... PASSED
Testing best-fit allocation... PASSED
...
=== All PhysicalMemory Tests Passed! ===
```

## Adding New Tests

To add a new test:

1. Create a new test file in the `tests/` directory
2. Follow the pattern used in existing test files
3. Add the test executable to `CMakeLists.txt`:

```cmake
add_executable(test_new_component
    tests/test_new_component.cpp
    src/path/to/source.cpp
)
target_include_directories(test_new_component
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)
```

4. Add to the `run_tests` target dependencies

## Debugging Failed Tests

If a test fails:

1. The assertion will show which test case failed
2. Check the test output for the specific assertion
3. Use a debugger to step through the failing test
4. Verify the component implementation matches expected behavior

## Notes

- Tests use simple assertions (no external test framework required)
- Each test is independent and can run standalone
- Tests are designed to be fast and comprehensive
- All tests should complete in under a second on modern hardware
