# Test Artifacts - Quick Reference

## ✅ Correct Directory Structure

```
memory-simulator/
└── test_artifacts/
    ├── README.md
    ├── TESTING_GUIDE.md
    ├── IMPLEMENTATION_SUMMARY.md
    ├── workloads/           # Input test patterns
    │   ├── memory_allocation_sequential.txt
    │   ├── buddy_allocator_stress.txt
    │   ├── cache_access_patterns.txt
    │   └── virtual_memory_fifo.txt
    ├── expected_outputs/    # Expected behavior documentation
    │   ├── memory_allocation_expected.txt
    │   ├── buddy_allocator_expected.txt
    │   ├── cache_access_expected.txt
    │   └── virtual_memory_expected.txt
    ├── logs/                # ✅ TEST OUTPUT LOGS ARE HERE
    │   ├── test_physical_memory.log (2 KB)
    │   ├── test_buddy_allocator.log (2 KB)
    │   ├── test_cache.log (2 KB)
    │   ├── test_virtual_memory.log (3 KB)
    │   ├── test_page_table.log (1 KB)
    │   ├── test_virtual_address.log (1 KB)
    │   └── test_runner.log (1 KB)
    └── scripts/             # Test automation scripts
        ├── run_all_tests.bat
        ├── run_all_tests.ps1
        └── validate_test_output.py
```

## 📍 Correct Log File Locations

All test logs are in: **`test_artifacts\logs\`**

| Test File | Log Location | Size |
|-----------|-------------|------|
| Physical Memory | `test_artifacts\logs\test_physical_memory.log` | 2 KB |
| Buddy Allocator | `test_artifacts\logs\test_buddy_allocator.log` | 2 KB |
| Cache | `test_artifacts\logs\test_cache.log` | 2 KB |
| Virtual Memory | `test_artifacts\logs\test_virtual_memory.log` | 3 KB |
| Page Table | `test_artifacts\logs\test_page_table.log` | 1 KB |
| Virtual Address | `test_artifacts\logs\test_virtual_address.log` | 1 KB |
| Test Runner | `test_artifacts\logs\test_runner.log` | 1 KB |

## 🚀 How to Regenerate Logs

### Method 1: Run Individual Tests
```powershell
cd build
.\test_physical_memory.exe | Tee-Object ..\test_artifacts\logs\test_physical_memory.log
.\test_cache.exe | Tee-Object ..\test_artifacts\logs\test_cache.log
.\test_virtual_memory.exe | Tee-Object ..\test_artifacts\logs\test_virtual_memory.log
```

### Method 2: Run All Tests at Once
```powershell
cd build
Get-ChildItem -Filter "test_*.exe" | ForEach-Object {
    Write-Host "Running $($_.Name)..."
    & $_.FullName | Out-File "..\test_artifacts\logs\$($_.BaseName).log"
}
```

### Method 3: Use the Provided Script
```powershell
cd test_artifacts\scripts
.\run_all_tests.ps1
```

## 📖 How to View Logs

### View in Terminal
```powershell
Get-Content test_artifacts\logs\test_cache.log
```

### View First 30 Lines
```powershell
Get-Content test_artifacts\logs\test_cache.log | Select-Object -First 30
```

### Search for Specific Pattern
```powershell
Select-String -Path test_artifacts\logs\*.log -Pattern "FAILED"
Select-String -Path test_artifacts\logs\*.log -Pattern "EXPECTED"
```

### Count Passed Tests
```powershell
(Select-String -Path test_artifacts\logs\*.log -Pattern "PASSED").Count
```

## ✅ All Logs Are Working

All 7 test log files have been successfully generated with detailed debug output showing:
- **[DEBUG]** - Configuration information
- **[STEP N]** - Test steps
- **[CALC]** - Calculations performed
- **[EXPECTED]** - Expected values
- **[ACTUAL]** - Actual values from system
- **[STATE]** - System state snapshots
- **PASSED/FAILED** - Test results

## 🔍 Sample Log Output

From `test_cache.log`:
```
Testing cache hit...
  [DEBUG] Creating cache: 1024 bytes, 64-byte lines
  [STEP 1] First access to address 0x1000
  [EXPECTED] First access should be a MISS
  [ACTUAL]   Result = MISS
  [DEBUG] Filling cache line for address 0x1000
  [STATE]    misses = 1, hits = 0
  [STEP 2] Second access to same address 0x1000
  [EXPECTED] Second access should be a HIT
  [ACTUAL]   Result = HIT
  [EXPECTED] hits = 1, misses = 1
  [ACTUAL]   hits = 1, misses = 1
PASSED
```

## ❌ Common Issues (FIXED)

### Issue: Nested test_artifacts directory
- **Problem**: Accidentally created `test_artifacts\test_artifacts\logs`
- **Solution**: ✅ **FIXED** - Removed nested directory
- **Correct path**: `test_artifacts\logs\`

### Issue: Empty log files
- **Problem**: Logs were in wrong location or not generated
- **Solution**: ✅ **FIXED** - All logs regenerated in correct location
- **Verification**: All logs now have content (1-3 KB each)

## 📊 Current Status

✅ All 7 test log files generated
✅ All logs contain detailed debug output
✅ All logs in correct location (`test_artifacts\logs\`)
✅ Incorrect nested directory removed
✅ Total log size: ~10 KB
✅ All tests PASSING

You can now view any log file in `test_artifacts\logs\` to see the detailed test output with expected vs actual comparisons!
