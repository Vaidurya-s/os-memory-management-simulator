# Quick Start Guide - Memory Simulator Tests

## Fastest Way to Build and Test

### Windows Users

#### Option 1: Batch Script (Easiest)
```cmd
build_and_test.bat
```

#### Option 2: PowerShell (Recommended)
```powershell
pwsh build_and_test.ps1
```

### Linux/macOS Users

#### Using PowerShell (Recommended)
```bash
pwsh build_and_test.ps1
```

#### Manual Build
```bash
mkdir build && cd build
cmake ..
cmake --build .
cmake --build . --target run_tests
```

## What Gets Built

After building, you'll have these test executables in the `build/` directory:

```
build/
├── test_physical_memory     # Tests physical memory allocator
├── test_buddy_allocator     # Tests buddy allocation system
├── test_cache               # Tests cache simulation
├── test_virtual_memory      # Tests virtual memory manager
├── test_page_table          # Tests page table
├── test_virtual_address     # Tests address decoder
└── test_runner              # Main test orchestrator
```

## Running Tests

### Run All Tests
```bash
cd build
cmake --build . --target run_tests
```

### Run Individual Tests
```bash
cd build

# Windows
test_physical_memory.exe
test_buddy_allocator.exe
test_cache.exe
test_virtual_memory.exe
test_page_table.exe
test_virtual_address.exe

# Linux/macOS
./test_physical_memory
./test_buddy_allocator
./test_cache
./test_virtual_memory
./test_page_table
./test_virtual_address
```

## Expected Output

Each test suite will output:
```
=== Running [Component] Tests ===
Testing [feature 1]... PASSED
Testing [feature 2]... PASSED
...
=== All [Component] Tests Passed! ===
```

## If a Test Fails

1. Note which specific test failed
2. Check the assertion message
3. Review the component's source code
4. Check the test implementation for expected behavior
5. Run the individual test in a debugger if needed

## Troubleshooting

### Build Fails
- Ensure CMake 3.10+ is installed
- Ensure C++17 compiler is available
- Check compiler version (GCC 7+, Clang 5+, MSVC 2017+)

### CMake Not Found
```bash
# Install CMake first
# Windows: Download from cmake.org
# Linux: sudo apt-get install cmake
# macOS: brew install cmake
```

### Compiler Not Found
```bash
# Windows: Install Visual Studio or MinGW
# Linux: sudo apt-get install build-essential
# macOS: xcode-select --install
```

## Building Without Tests

If you only want the main simulator:
```bash
cmake .. -DBUILD_TESTS=OFF
cmake --build .
```

## Test Coverage Summary

- **Physical Memory**: 11 test cases
- **Buddy Allocator**: 12 test cases
- **Cache**: 12 test cases
- **Virtual Memory**: 12 test cases
- **Page Table**: 10 test cases
- **Virtual Address**: 10 test cases

**Total: 67+ comprehensive test cases**

## Next Steps

After tests pass:
1. Read [tests/README.md](tests/README.md) for detailed test documentation
2. Read [docs/README.md](docs/README.md) for component documentation
3. Check [docs/USAGE.md](docs/USAGE.md) for usage examples
4. Explore the source code in `src/` and `include/`

## Quick Reference

```bash
# Full build and test cycle
mkdir build && cd build
cmake ..
cmake --build .
cmake --build . --target run_tests

# Clean build
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .

# Rebuild after changes
cd build
cmake --build .

# Run specific test
cd build
./test_cache  # or test_cache.exe on Windows
```

## Success Indicators

✅ All files compile without errors
✅ All tests show "PASSED"
✅ Final summary shows "All tests PASSED!"
✅ No assertion failures
✅ No segmentation faults or crashes

## Common Issues

| Issue | Solution |
|-------|----------|
| CMake version too old | Upgrade to CMake 3.10+ |
| C++17 not supported | Update compiler to GCC 7+, Clang 5+, or MSVC 2017+ |
| Tests fail | Check component implementation, may need fixes |
| Build directory issues | Delete `build/` and rebuild from scratch |
| Permission denied | Run with appropriate permissions (sudo if needed) |

## Getting Help

1. Check [tests/README.md](tests/README.md) for test documentation
2. Check [TESTING_SUMMARY.md](TESTING_SUMMARY.md) for implementation details
3. Review component source code for expected behavior
4. Check CMakeLists.txt for build configuration

---

**Happy Testing! 🧪**
