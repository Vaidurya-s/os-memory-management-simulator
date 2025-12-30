# Test Artifacts Summary

## Overview
This directory contains a comprehensive testing framework for the Memory Simulator with:
- **Debug-instrumented tests** with expected vs actual output comparisons
- **Test workloads** simulating realistic memory access patterns
- **Expected outputs** with detailed correctness criteria
- **Validation scripts** for automated output verification
- **Log files** capturing test execution traces

## What Was Added

### 1. Enhanced Test Files (Added Debug Prints)
All test files in `/tests/` now include:
- `[DEBUG]` - Configuration and setup information
- `[STEP N]` - Individual test steps
- `[CALC]` - Mathematical calculations shown
- `[EXPECTED]` - Calculated expected values
- `[ACTUAL]` - Values returned by the system
- `[STATE]` - Current system state snapshots
- `[RESULT]` - Pass/Fail indicators

**Modified files:**
- `tests/test_physical_memory.cpp`
- `tests/test_buddy_allocator.cpp`
- `tests/test_cache.cpp`
- `tests/test_virtual_memory.cpp`
- `tests/test_page_table.cpp`

### 2. Test Artifacts Directory Structure
```
test_artifacts/
├── README.md                          # Complete testing documentation
├── workloads/                         # Input test workloads
│   ├── memory_allocation_sequential.txt
│   ├── buddy_allocator_stress.txt
│   ├── cache_access_patterns.txt
│   └── virtual_memory_fifo.txt
├── expected_outputs/                  # Expected behavior documentation
│   ├── memory_allocation_expected.txt
│   ├── buddy_allocator_expected.txt
│   ├── cache_access_expected.txt
│   └── virtual_memory_expected.txt
├── logs/                              # Test execution logs
│   └── (generated when tests run)
└── scripts/                           # Automation scripts
    ├── run_all_tests.bat             # Windows batch script
    ├── run_all_tests.ps1             # PowerShell script
    └── validate_test_output.py       # Python validation tool
```

## Key Features

### Debug Output Example
When you run tests, you'll see detailed output like:

```
Testing first-fit allocation...
  [DEBUG] Creating PhysicalMemory with size 1024 bytes
  [STEP 1] Allocating 100 bytes using first-fit
  [EXPECTED] id1 >= 0 (allocation successful)
  [ACTUAL]   id1 = 1
  [EXPECTED] used_memory = 100
  [ACTUAL]   used_memory = 100
  [EXPECTED] free_memory = 1024 - 100 = 924
  [ACTUAL]   free_memory = 924
PASSED
```

### Expected Behavior Calculations

Each test now:
1. **Calculates** expected values from input parameters
2. **Displays** the calculation (e.g., "1024 - 100 = 924")
3. **Shows** actual system output
4. **Verifies** match between expected and actual
5. **Asserts** correctness

## Test Workloads

### 1. Memory Allocation Sequential (`memory_allocation_sequential.txt`)
- Tests first-fit, best-fit, worst-fit strategies
- Creates and resolves fragmentation
- Validates coalescing behavior
- **Expected**: 2048 bytes total, proper allocation tracking

### 2. Buddy Allocator Stress (`buddy_allocator_stress.txt`)
- Tests power-of-2 rounding (100→128, 513→1024)
- Validates splitting and coalescing
- Stress tests with multiple allocations
- **Expected**: All sizes power of 2, buddy pairs coalesce

### 3. Cache Access Patterns (`cache_access_patterns.txt`)
- Sequential access (spatial locality)
- Temporal locality (repeated accesses)
- Conflict misses (same set mapping)
- Strided access patterns
- **Expected**: Hit ratio 60-70%, proper set indexing

### 4. Virtual Memory FIFO (`virtual_memory_fifo.txt`)
- Page fault behavior
- FIFO replacement policy
- Offset preservation verification
- Working set analysis
- **Expected**: 20 faults on 30 accesses, oldest-first eviction

## Expected Outputs

Each workload has corresponding expected output documentation showing:
- **Detailed calculations** for each operation
- **State transitions** (before/after each step)
- **Correctness criteria** (invariants that must hold)
- **Formula explanations** (how values are computed)

Example from cache:
```
Set calculation: (address / line_size) % num_sets
addr1 = 0x0000 → set 0
addr2 = 0x0400 → set 0 (conflicts with addr1)
```

## Correctness Criteria

### Physical Memory
✓ used_memory + free_memory = total_memory (always)
✓ Allocation IDs unique and ≥ 0
✓ 0.0 ≤ fragmentation ≤ 1.0

### Buddy Allocator
✓ All allocations are power of 2
✓ Address alignment: addr % size == 0
✓ Buddy pairs: addr1 XOR addr2 = size
✓ Complete coalescing to original block

### Cache
✓ 0 ≤ hit_ratio ≤ 1.0
✓ set_index < num_sets
✓ Conflict detection via set mapping
✓ Offset < line_size

### Virtual Memory
✓ Offset preservation: vaddr_offset == paddr_offset
✓ Page faults monotonically increasing
✓ frame_number < total_frames
✓ FIFO order maintained

## How to Use

### Run All Tests
```bash
# Windows Command Prompt
cd test_artifacts\scripts
run_all_tests.bat

# PowerShell
cd test_artifacts\scripts
.\run_all_tests.ps1
```

This will:
1. Run all 6 test suites
2. Save output to `logs/` directory
3. Display pass/fail summary

### Manual Test Execution
```bash
cd build
.\test_physical_memory.exe
.\test_buddy_allocator.exe
.\test_cache.exe
.\test_virtual_memory.exe
.\test_page_table.exe
.\test_virtual_address.exe
```

### Validate Output
```bash
cd test_artifacts
python scripts\validate_test_output.py logs\test_physical_memory.log
```

The validator checks:
- Expected vs Actual value matches
- Invariants (e.g., total memory constant)
- Hit ratios in valid range
- Offset preservation
- Power-of-2 requirements

## Test Coverage Matrix

| Component | Tests | Key Areas |
|-----------|-------|-----------|
| Physical Memory | 11 | First/Best/Worst fit, fragmentation, coalescing |
| Buddy Allocator | 12 | Power-of-2, splitting, coalescing, stress |
| Cache | 12 | Hit/miss, replacement, conflict, patterns |
| Virtual Memory | 11 | Translation, FIFO/LRU, faults, thrashing |
| Page Table | 10 | Entries, timestamps, dirty/valid bits |
| Virtual Address | 8 | Bit extraction, page/offset calculation |

## Sample Test Output

See `logs/test_physical_memory.log` for complete example showing:
- Configuration details
- Step-by-step execution
- Expected value calculations  
- Actual values from system
- State changes
- Pass/Fail results

## Validation Criteria Summary

All tests verify:
1. **Memory Invariants** - No leaks, proper accounting
2. **Correctness** - Expected values match actual
3. **Alignment** - Address alignment requirements
4. **Bounds** - All indices/offsets within valid ranges
5. **Consistency** - State transitions are valid
6. **Performance** - Expected complexity characteristics

## Next Steps

1. **Run tests**: Execute test suites to see debug output
2. **Review logs**: Examine generated log files in `logs/`
3. **Compare outputs**: Match against expected outputs in `expected_outputs/`
4. **Validate**: Use Python validator to check correctness
5. **Analyze**: Use workload patterns to understand behavior

## Benefits

✓ **Transparency** - See exactly what's being tested and why
✓ **Debugging** - Detailed output pinpoints failures
✓ **Learning** - Understand memory management algorithms
✓ **Validation** - Automated checking of correctness
✓ **Documentation** - Self-documenting test behavior

## Files Generated

When you run the full test suite:
- `logs/test_physical_memory.log` - ~15 KB
- `logs/test_buddy_allocator.log` - ~18 KB
- `logs/test_cache.log` - ~12 KB
- `logs/test_virtual_memory.log` - ~20 KB
- `logs/test_page_table.log` - ~8 KB
- `logs/test_virtual_address.log` - ~6 KB

Total: ~80 KB of detailed test traces
