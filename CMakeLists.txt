cmake_minimum_required(VERSION 3.10)

project(memory_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Main executable
add_executable(memsim
    src/main.cpp
    src/allocator/PhysicalMemory.cpp
    src/buddy/BuddyAllocator.cpp
    src/cache/DirectMappedCache.cpp
    src/cache/CacheHierarchy.cpp
    src/virtual_memory/PageTable.cpp
    src/virtual_memory/VirtualAddress.cpp
    src/virtual_memory/VirtualMemoryManager.cpp
)

target_include_directories(memsim
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# ==================================
# Test Executables
# ==================================

option(BUILD_TESTS "Build test executables" ON)

if(BUILD_TESTS)
    # Test for PhysicalMemory
    add_executable(test_physical_memory
        tests/test_physical_memory.cpp
        src/allocator/PhysicalMemory.cpp
    )
    target_include_directories(test_physical_memory
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Test for BuddyAllocator
    add_executable(test_buddy_allocator
        tests/test_buddy_allocator.cpp
        src/buddy/BuddyAllocator.cpp
    )
    target_include_directories(test_buddy_allocator
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Test for DirectMappedCache
    add_executable(test_cache
        tests/test_cache.cpp
        src/cache/DirectMappedCache.cpp
    )
    target_include_directories(test_cache
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Test for VirtualMemoryManager
    add_executable(test_virtual_memory
        tests/test_virtual_memory.cpp
        src/virtual_memory/VirtualMemoryManager.cpp
        src/virtual_memory/PageTable.cpp
        src/virtual_memory/VirtualAddress.cpp
    )
    target_include_directories(test_virtual_memory
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Test for PageTable
    add_executable(test_page_table
        tests/test_page_table.cpp
        src/virtual_memory/PageTable.cpp
    )
    target_include_directories(test_page_table
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Test for VirtualAddressDecoder
    add_executable(test_virtual_address
        tests/test_virtual_address.cpp
        src/virtual_memory/VirtualAddress.cpp
    )
    target_include_directories(test_virtual_address
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Test runner
    add_executable(test_runner
        tests/test_runner.cpp
    )
    target_include_directories(test_runner
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Test for CLI integration
    add_executable(test_cli
        tests/test_cli.cpp
        src/allocator/PhysicalMemory.cpp
        src/buddy/BuddyAllocator.cpp
        src/cache/DirectMappedCache.cpp
        src/cache/CacheHierarchy.cpp
        src/virtual_memory/PageTable.cpp
        src/virtual_memory/VirtualAddress.cpp
        src/virtual_memory/VirtualMemoryManager.cpp
    )
    target_include_directories(test_cli
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
    )

    # Custom target to run all tests
    add_custom_target(run_tests
        COMMAND test_physical_memory
        COMMAND test_buddy_allocator
        COMMAND test_cache
        COMMAND test_virtual_memory
        COMMAND test_page_table
        COMMAND test_virtual_address
        COMMAND test_cli
        DEPENDS 
            test_physical_memory
            test_buddy_allocator
            test_cache
            test_virtual_memory
            test_page_table
            test_virtual_address
            test_cli
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running all tests..."
    )
endif()
