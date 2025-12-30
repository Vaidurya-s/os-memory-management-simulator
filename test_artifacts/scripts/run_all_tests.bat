@echo off
REM Windows Batch Script to Run All Tests and Capture Output
REM Saves test output to logs directory for validation

echo ========================================
echo   Running Memory Simulator Test Suite
echo ========================================
echo.

set BUILD_DIR=..\build
set LOG_DIR=..\test_artifacts\logs

REM Create logs directory if it doesn't exist
if not exist "%LOG_DIR%" mkdir "%LOG_DIR%"

echo [1/6] Running Physical Memory Tests...
%BUILD_DIR%\test_physical_memory.exe > %LOG_DIR%\test_physical_memory.log 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   ✓ Physical Memory Tests PASSED
) else (
    echo   ✗ Physical Memory Tests FAILED
)

echo [2/6] Running Buddy Allocator Tests...
%BUILD_DIR%\test_buddy_allocator.exe > %LOG_DIR%\test_buddy_allocator.log 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   ✓ Buddy Allocator Tests PASSED
) else (
    echo   ✗ Buddy Allocator Tests FAILED
)

echo [3/6] Running Cache Tests...
%BUILD_DIR%\test_cache.exe > %LOG_DIR%\test_cache.log 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   ✓ Cache Tests PASSED
) else (
    echo   ✗ Cache Tests FAILED
)

echo [4/6] Running Virtual Memory Tests...
%BUILD_DIR%\test_virtual_memory.exe > %LOG_DIR%\test_virtual_memory.log 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   ✓ Virtual Memory Tests PASSED
) else (
    echo   ✗ Virtual Memory Tests FAILED
)

echo [5/6] Running Page Table Tests...
%BUILD_DIR%\test_page_table.exe > %LOG_DIR%\test_page_table.log 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   ✓ Page Table Tests PASSED
) else (
    echo   ✗ Page Table Tests FAILED
)

echo [6/6] Running Virtual Address Tests...
%BUILD_DIR%\test_virtual_address.exe > %LOG_DIR%\test_virtual_address.log 2>&1
if %ERRORLEVEL% EQU 0 (
    echo   ✓ Virtual Address Tests PASSED
) else (
    echo   ✗ Virtual Address Tests FAILED
)

echo.
echo ========================================
echo Test outputs saved to: %LOG_DIR%
echo ========================================
echo.
echo To validate outputs against expected behavior:
echo   python scripts\validate_test_output.py logs\test_physical_memory.log
echo.

pause
