cmake_minimum_required(VERSION 3.10)

project(memory_simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable warnings
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

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
