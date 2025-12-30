#!/usr/bin/env pwsh
# PowerShell Script to Run All Tests and Capture Output
# Saves test output to logs directory for validation

Write-Host "========================================"
Write-Host "  Running Memory Simulator Test Suite"
Write-Host "========================================"
Write-Host ""

$BUILD_DIR = "..\build"
$LOG_DIR = "..\test_artifacts\logs"

# Create logs directory if it doesn't exist
if (-not (Test-Path $LOG_DIR)) {
    New-Item -ItemType Directory -Path $LOG_DIR | Out-Null
}

$tests = @(
    @{Name="Physical Memory"; Exe="test_physical_memory.exe"; Log="test_physical_memory.log"},
    @{Name="Buddy Allocator"; Exe="test_buddy_allocator.exe"; Log="test_buddy_allocator.log"},
    @{Name="Cache"; Exe="test_cache.exe"; Log="test_cache.log"},
    @{Name="Virtual Memory"; Exe="test_virtual_memory.exe"; Log="test_virtual_memory.log"},
    @{Name="Page Table"; Exe="test_page_table.exe"; Log="test_page_table.log"},
    @{Name="Virtual Address"; Exe="test_virtual_address.exe"; Log="test_virtual_address.log"}
)

$total = $tests.Count
$passed = 0
$failed = 0

for ($i = 0; $i -lt $tests.Count; $i++) {
    $test = $tests[$i]
    $num = $i + 1
    
    Write-Host "[$num/$total] Running $($test.Name) Tests..." -NoNewline
    
    $exePath = Join-Path $BUILD_DIR $test.Exe
    $logPath = Join-Path $LOG_DIR $test.Log
    
    if (Test-Path $exePath) {
        & $exePath > $logPath 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host " ✓ PASSED" -ForegroundColor Green
            $passed++
        } else {
            Write-Host " ✗ FAILED" -ForegroundColor Red
            $failed++
        }
    } else {
        Write-Host " ✗ NOT FOUND" -ForegroundColor Yellow
        Write-Host "  (Expected: $exePath)"
    }
}

Write-Host ""
Write-Host "========================================"
Write-Host "           Test Summary"
Write-Host "========================================"
Write-Host "Total:  $total"
Write-Host "Passed: $passed" -ForegroundColor Green
Write-Host "Failed: $failed" -ForegroundColor $(if ($failed -gt 0) { "Red" } else { "Green" })
Write-Host "========================================"
Write-Host ""
Write-Host "Test outputs saved to: $LOG_DIR"
Write-Host ""
Write-Host "To validate outputs against expected behavior:"
Write-Host '  python scripts\validate_test_output.py logs\test_physical_memory.log'
Write-Host ""
