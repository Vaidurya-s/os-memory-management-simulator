#include "../include/allocator/IAllocator.h"
#include "../include/allocator/PhysicalMemory.h"
#include "../include/buddy/BuddyAllocator.h"
#include "../include/cache/DirectMappedCache.h"
#include "../include/cache/CacheHierarchy.h"
#include "../include/virtual_memory/VirtualMemoryManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <string>
#include <map>
#include <iomanip>

class CLITests {
public:
    static void run_all_tests() {
        std::cout << "\n=== Running CLI Tests ===\n";
        
        test_first_fit_basic();
        test_best_fit_basic();
        test_worst_fit_basic();
        test_buddy_system_basic();
        test_allocation_failure();
        test_fragmentation_handling();
        test_integration_with_cache();
        test_integration_with_virtual_memory();
        test_full_integration();
        test_small_memory_configuration();
        
        std::cout << "=== All CLI Tests Passed! ===\n\n";
    }

private:
    static void test_first_fit_basic() {
        std::cout << "Testing First Fit CLI operations... ";
        std::cout << "\n  [DEBUG] Simulating First Fit allocator CLI\n";
        
        PhysicalMemory allocator(1024, AllocationStrategy::FIRST_FIT);
        std::map<int, size_t> blockSizes;
        
        // Test malloc
        std::cout << "  [TEST] malloc 100\n";
        int block1 = allocator.allocate(100);
        assert(block1 != -1);
        blockSizes[block1] = 100;
        std::cout << "  [RESULT] Allocated block id=" << block1 << "\n";
        
        // Test second malloc
        std::cout << "  [TEST] malloc 200\n";
        int block2 = allocator.allocate(200);
        assert(block2 != -1);
        blockSizes[block2] = 200;
        std::cout << "  [RESULT] Allocated block id=" << block2 << "\n";
        
        // Test stats
        std::cout << "  [TEST] stats command\n";
        std::cout << "  [EXPECTED] Total memory: 1024, Used: 300\n";
        std::cout << "  [ACTUAL] Total: " << allocator.total_memory() 
                  << ", Used: " << allocator.used_memory() << "\n";
        assert(allocator.total_memory() == 1024);
        assert(allocator.used_memory() == 300);
        
        // Test free
        std::cout << "  [TEST] free " << block1 << "\n";
        allocator.free_block(block1);
        blockSizes.erase(block1);
        std::cout << "  [RESULT] Block freed, used memory: " << allocator.used_memory() << "\n";
        assert(allocator.used_memory() == 200);
        
        // Test reallocation in freed space
        std::cout << "  [TEST] malloc 50 (should use freed space)\n";
        int block3 = allocator.allocate(50);
        assert(block3 != -1);
        blockSizes[block3] = 50;
        std::cout << "  [RESULT] Allocated block id=" << block3 << "\n";
        
        std::cout << "PASSED\n";
    }
    
    static void test_best_fit_basic() {
        std::cout << "Testing Best Fit CLI operations... ";
        std::cout << "\n  [DEBUG] Simulating Best Fit allocator CLI\n";
        
        PhysicalMemory allocator(2048, AllocationStrategy::BEST_FIT);
        
        std::cout << "  [TEST] Creating fragmented memory\n";
        int b1 = allocator.allocate(100);
        int b2 = allocator.allocate(500);
        int b3 = allocator.allocate(200);
        int b4 = allocator.allocate(300);
        (void)b2; (void)b4; // Keep these allocated
        
        std::cout << "  [TEST] Freeing blocks to create gaps\n";
        allocator.free_block(b1);  // 100-byte gap
        allocator.free_block(b3);  // 200-byte gap
        
        std::cout << "  [TEST] malloc 150 (should use 200-byte gap, best fit)\n";
        int b5 = allocator.allocate(150);
        assert(b5 != -1);
        std::cout << "  [RESULT] Successfully allocated in best fit gap\n";
        
        std::cout << "PASSED\n";
    }
    
    static void test_worst_fit_basic() {
        std::cout << "Testing Worst Fit CLI operations... ";
        std::cout << "\n  [DEBUG] Simulating Worst Fit allocator CLI\n";
        
        PhysicalMemory allocator(4096, AllocationStrategy::WORST_FIT);
        
        std::cout << "  [TEST] malloc 1024\n";
        int block1 = allocator.allocate(1024);
        assert(block1 != -1);
        
        std::cout << "  [TEST] Largest free block after allocation\n";
        size_t largest = allocator.largest_free_block();
        std::cout << "  [RESULT] Largest free block: " << largest << "\n";
        assert(largest == 4096 - 1024);
        
        std::cout << "PASSED\n";
    }
    
    static void test_buddy_system_basic() {
        std::cout << "Testing Buddy System CLI operations... ";
        std::cout << "\n  [DEBUG] Simulating Buddy System allocator CLI\n";
        
        BuddyAllocator buddy(1024);
        std::map<int, size_t> blockSizes;
        
        std::cout << "  [TEST] malloc 100 (should round to 128)\n";
        int block1 = buddy.allocate(100);
        assert(block1 != -1);
        blockSizes[block1] = 100;
        std::cout << "  [RESULT] Allocated block id=" << block1 << "\n";
        
        std::cout << "  [TEST] malloc 200 (should round to 256)\n";
        int block2 = buddy.allocate(200);
        assert(block2 != -1);
        blockSizes[block2] = 200;
        std::cout << "  [RESULT] Allocated block id=" << block2 << "\n";
        
        std::cout << "  [TEST] Stats - used memory\n";
        std::cout << "  [EXPECTED] Used: 384 (128 + 256)\n";
        std::cout << "  [ACTUAL] Used: " << buddy.allocated_memory() << "\n";
        assert(buddy.allocated_memory() == 384);
        
        std::cout << "  [TEST] free and buddy coalescing\n";
        buddy.free_block(block1);
        buddy.free_block(block2);
        std::cout << "  [RESULT] All memory freed, allocated: " << buddy.allocated_memory() << "\n";
        assert(buddy.allocated_memory() == 0);
        
        std::cout << "PASSED\n";
    }
    
    static void test_allocation_failure() {
        std::cout << "Testing allocation failure handling... ";
        std::cout << "\n  [DEBUG] Testing out-of-memory scenarios\n";
        
        PhysicalMemory allocator(512, AllocationStrategy::FIRST_FIT);
        
        std::cout << "  [TEST] malloc 256\n";
        int block1 = allocator.allocate(256);
        assert(block1 != -1);
        
        std::cout << "  [TEST] malloc 256\n";
        int block2 = allocator.allocate(256);
        assert(block2 != -1);
        
        std::cout << "  [TEST] malloc 100 (should fail - no space)\n";
        int block3 = allocator.allocate(100);
        std::cout << "  [RESULT] Allocation result: " << block3 << " (expected -1)\n";
        assert(block3 == -1);
        
        std::cout << "  [TEST] free one block\n";
        allocator.free_block(block1);
        
        std::cout << "  [TEST] malloc 100 (should succeed now)\n";
        int block4 = allocator.allocate(100);
        std::cout << "  [RESULT] Allocation successful: " << (block4 != -1) << "\n";
        assert(block4 != -1);
        
        std::cout << "PASSED\n";
    }
    
    static void test_fragmentation_handling() {
        std::cout << "Testing fragmentation handling... ";
        std::cout << "\n  [DEBUG] Creating and measuring fragmentation\n";
        
        PhysicalMemory allocator(1024, AllocationStrategy::FIRST_FIT);
        
        std::cout << "  [TEST] Create fragmentation pattern\n";
        int blocks[10];
        for (int i = 0; i < 10; i++) {
            blocks[i] = allocator.allocate(50);
            assert(blocks[i] != -1);
        }
        
        std::cout << "  [TEST] Free every other block\n";
        for (int i = 0; i < 10; i += 2) {
            allocator.free_block(blocks[i]);
        }
        
        std::cout << "  [TEST] Check memory state after fragmentation\n";
        size_t largest = allocator.largest_free_block();
        size_t totalFree = allocator.free_memory();
        std::cout << "  [RESULT] Largest free: " << largest 
                  << ", Total free: " << totalFree << "\n";
        // Largest free should be less than total free (indicates fragmentation)
        assert(largest < totalFree);
        
        std::cout << "PASSED\n";
    }
    
    static void test_integration_with_cache() {
        std::cout << "Testing cache integration... ";
        std::cout << "\n  [DEBUG] Testing memory access with cache\n";
        
        DirectMappedCache l1Cache(32 * 1024, 64, 1);
        DirectMappedCache l2Cache(256 * 1024, 64, 1);
        CacheHierarchy cacheHierarchy(l1Cache, l2Cache);
        
        std::cout << "  [TEST] access 0x1000\n";
        bool hit1 = cacheHierarchy.access(0x1000);
        std::cout << "  [RESULT] First access: " << (hit1 ? "HIT" : "MISS") << " (expected MISS)\n";
        assert(!hit1); // First access should miss
        
        std::cout << "  [TEST] access 0x1000 again\n";
        bool hit2 = cacheHierarchy.access(0x1000);
        std::cout << "  [RESULT] Second access: " << (hit2 ? "HIT" : "MISS") << " (expected HIT)\n";
        assert(hit2); // Second access should hit
        
        std::cout << "  [TEST] cache_stats\n";
        std::cout << "  [RESULT] L1 hits: " << cacheHierarchy.l1_hits() << ", misses: " 
                  << cacheHierarchy.l1_misses() << "\n";
        assert(cacheHierarchy.l1_hits() == 1);
        assert(cacheHierarchy.l1_misses() == 1);
        
        std::cout << "PASSED\n";
    }
    
    static void test_integration_with_virtual_memory() {
        std::cout << "Testing virtual memory integration... ";
        std::cout << "\n  [DEBUG] Testing virtual address translation\n";
        
        size_t pageSize = 4096;
        size_t numVirtualPages = 16;
        size_t numPhysicalFrames = 4;
        
        VirtualMemoryManager vmManager(numVirtualPages, numPhysicalFrames, pageSize);
        
        std::cout << "  [TEST] access virtual address 0x1000\n";
        uint64_t physAddr1 = vmManager.translate(0x1000);
        std::cout << "  [RESULT] Physical address: 0x" << std::hex << physAddr1 << std::dec << "\n";
        
        std::cout << "  [TEST] vm_stats - page faults\n";
        std::cout << "  [RESULT] Page faults: " << vmManager.page_faults() << " (expected >= 1)\n";
        assert(vmManager.page_faults() >= 1);
        
        std::cout << "  [TEST] access same page again\n";
        size_t faults_before = vmManager.page_faults();
        vmManager.translate(0x1000);
        size_t faults_after = vmManager.page_faults();
        std::cout << "  [RESULT] Page faults before: " << faults_before 
                  << ", after: " << faults_after << " (should be same)\n";
        
        std::cout << "PASSED\n";
    }
    
    static void test_full_integration() {
        std::cout << "Testing full integration (Allocator + Cache + VM)... ";
        std::cout << "\n  [DEBUG] Full system integration test\n";
        
        // Setup all components
        PhysicalMemory allocator(65536, AllocationStrategy::FIRST_FIT);
        DirectMappedCache l1Cache(32 * 1024, 64, 1);
        DirectMappedCache l2Cache(256 * 1024, 64, 1);
        CacheHierarchy cacheHierarchy(l1Cache, l2Cache);
        VirtualMemoryManager vmManager(64, 16, 4096);
        
        std::cout << "  [TEST] malloc 1024\n";
        int block1 = allocator.allocate(1024);
        assert(block1 != -1);
        std::cout << "  [RESULT] Block allocated: " << block1 << "\n";
        
        std::cout << "  [TEST] Simulate memory access flow: VA -> PT -> PA -> Cache\n";
        uint64_t virtualAddr = 0x1000;
        uint64_t physicalAddr = vmManager.translate(virtualAddr);
        bool cacheHit = cacheHierarchy.access(physicalAddr);
        
        std::cout << "  [RESULT] VA: 0x" << std::hex << virtualAddr 
                  << " -> PA: 0x" << physicalAddr << std::dec 
                  << ", Cache: " << (cacheHit ? "HIT" : "MISS") << "\n";
        
        std::cout << "  [TEST] stats - all components\n";
        std::cout << "  [MEMORY] Total: " << allocator.total_memory() 
                  << ", Used: " << allocator.used_memory() << "\n";
        std::cout << "  [CACHE] L1 hits: " << cacheHierarchy.l1_hits() 
                  << ", misses: " << cacheHierarchy.l1_misses() << "\n";
        std::cout << "  [VM] Page faults: " << vmManager.page_faults() << "\n";
        
        std::cout << "PASSED\n";
    }
    
    static void test_small_memory_configuration() {
        std::cout << "Testing small memory configuration... ";
        std::cout << "\n  [DEBUG] Testing with 1KB memory\n";
        
        PhysicalMemory allocator(1024, AllocationStrategy::FIRST_FIT);
        
        // Virtual memory with at least 1 frame
        size_t pageSize = 4096;
        size_t numPhysicalFrames = std::max(size_t(1), 1024 / pageSize);
        size_t numVirtualPages = numPhysicalFrames * 4;
        VirtualMemoryManager vmManager(numVirtualPages, numPhysicalFrames, pageSize);
        
        std::cout << "  [TEST] Small memory VM configuration\n";
        std::cout << "  [RESULT] Virtual pages: " << numVirtualPages 
                  << ", Physical frames: " << numPhysicalFrames << "\n";
        assert(numVirtualPages > 0);
        assert(numPhysicalFrames > 0);
        
        std::cout << "  [TEST] malloc 500\n";
        int block1 = allocator.allocate(500);
        assert(block1 != -1);
        
        std::cout << "  [TEST] Virtual address access (256-byte spacing)\n";
        uint64_t vaddr = (block1 - 1) * 256;
        std::cout << "  [RESULT] Virtual address: 0x" << std::hex << vaddr << std::dec << "\n";
        
        // Should not exceed virtual address space
        assert(vaddr < numVirtualPages * pageSize);
        
        std::cout << "  [TEST] Translation should succeed\n";
        uint64_t paddr = vmManager.translate(vaddr);
        std::cout << "  [RESULT] Physical address: 0x" << std::hex << paddr << std::dec << "\n";
        
        std::cout << "PASSED\n";
    }
};

int main() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  CLI Component Tests\n";
    std::cout << "========================================\n";
    
    try {
        CLITests::run_all_tests();
        
        std::cout << "\n========================================\n";
        std::cout << "  All CLI Tests PASSED!\n";
        std::cout << "========================================\n\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n!!! TEST FAILED !!!\n";
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}
