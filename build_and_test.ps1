#!/usr/bin/env pwsh
# Build and Test Script for Memory Simulator
# PowerShell script (cross-platform)

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Memory Simulator - Build and Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Create build directory if it doesn't exist
if (-not (Test-Path "build")) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Navigate to build directory
Push-Location build

try {
    # Configure with CMake
    Write-Host ""
    Write-Host "Configuring project with CMake..." -ForegroundColor Yellow
    cmake ..
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed!"
    }

    # Build the project
    Write-Host ""
    Write-Host "Building project..." -ForegroundColor Yellow
    cmake --build .
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed!"
    }

    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""

    # Ask user if they want to run tests
    $runTests = Read-Host "Run all tests? (Y/N)"
    if ($runTests -eq "Y" -or $runTests -eq "y") {
        Write-Host ""
        Write-Host "Running all tests..." -ForegroundColor Cyan
        Write-Host "========================================" -ForegroundColor Cyan
        
        $testExecutables = @(
            @{Name="Physical Memory"; Exe="test_physical_memory"},
            @{Name="Buddy Allocator"; Exe="test_buddy_allocator"},
            @{Name="Cache"; Exe="test_cache"},
            @{Name="Virtual Memory"; Exe="test_virtual_memory"},
            @{Name="Page Table"; Exe="test_page_table"},
            @{Name="Virtual Address"; Exe="test_virtual_address"},
            @{Name="CLI Integration"; Exe="test_cli"}
        )
        
        $testNum = 1
        $totalTests = $testExecutables.Count
        $passedTests = 0
        $failedTests = 0
        
        foreach ($test in $testExecutables) {
            Write-Host ""
            Write-Host "[$testNum/$totalTests] Running $($test.Name) Tests..." -ForegroundColor Yellow
            
            $exeName = if ($IsWindows -or $env:OS -match "Windows") {
                "$($test.Exe).exe"
            } else {
                $test.Exe
            }
            
            & ".\$exeName"
            if ($LASTEXITCODE -eq 0) {
                $passedTests++
            } else {
                $failedTests++
            }
            
            $testNum++
        }
        
        Write-Host ""
        Write-Host "========================================" -ForegroundColor Cyan
        Write-Host "Test Summary" -ForegroundColor Cyan
        Write-Host "========================================" -ForegroundColor Cyan
        Write-Host "Total: $totalTests | Passed: $passedTests | Failed: $failedTests"
        
        if ($failedTests -eq 0) {
            Write-Host "All tests PASSED!" -ForegroundColor Green
        } else {
            Write-Host "Some tests FAILED!" -ForegroundColor Red
        }
        Write-Host "========================================" -ForegroundColor Cyan
    }
}
catch {
    Write-Host ""
    Write-Host "Error: $_" -ForegroundColor Red
    Pop-Location
    exit 1
}
finally {
    Pop-Location
}

Write-Host ""
Write-Host "Done!" -ForegroundColor Green
