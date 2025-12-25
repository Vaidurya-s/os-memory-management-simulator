@echo off
REM Build and Test Script for Memory Simulator
REM Windows batch script

echo ========================================
echo Memory Simulator - Build and Test
echo ========================================
echo.

REM Check if build directory exists
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

REM Navigate to build directory
cd build

REM Configure with CMake
echo.
echo Configuring project with CMake...
cmake ..
if %errorlevel% neq 0 (
    echo Error: CMake configuration failed!
    cd ..
    exit /b 1
)

REM Build the project
echo.
echo Building project...
cmake --build .
if %errorlevel% neq 0 (
    echo Error: Build failed!
    cd ..
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.

REM Ask user if they want to run tests
set /p run_tests="Run all tests? (Y/N): "
if /i "%run_tests%"=="Y" (
    echo.
    echo Running all tests...
    echo ========================================
    
    echo.
    echo [1/6] Running Physical Memory Tests...
    test_physical_memory.exe
    
    echo.
    echo [2/6] Running Buddy Allocator Tests...
    test_buddy_allocator.exe
    
    echo.
    echo [3/6] Running Cache Tests...
    test_cache.exe
    
    echo.
    echo [4/6] Running Virtual Memory Tests...
    test_virtual_memory.exe
    
    echo.
    echo [5/6] Running Page Table Tests...
    test_page_table.exe
    
    echo.
    echo [6/6] Running Virtual Address Tests...
    test_virtual_address.exe
    
    echo.
    echo ========================================
    echo All tests completed!
    echo ========================================
)

cd ..
echo.
echo Done!
pause
