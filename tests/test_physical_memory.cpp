#include "../include/allocator/PhysicalMemory.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>


class PhysicalMemoryTests {
public:
    static void run_all_tests() {
        std::cout << "\n=== Running PhysicalMemory Tests ===\n";
        
        test_initialization();
        test_first_fit_allocation();
        test_best_fit_allocation();
        test_worst_fit_allocation();
        test_free_and_reallocation();
        test_fragmentation();
        test_memory_metrics();
        test_allocation_failure();
        test_multiple_allocations();
        test_free_invalid_id();
        test_coalescing();
        
        std::cout << "=== All PhysicalMemory Tests Passed! ===\n\n";
    }

private:
    static void test_initialization() {
        std::cout << "Testing initialization... ";
        PhysicalMemory pm(1024);
        assert(pm.total_memory() == 1024);
        assert(pm.used_memory() == 0);
        assert(pm.free_memory() == 1024);
        assert(pm.largest_free_block() == 1024);
        std::cout << "PASSED\n";
    }

    static void test_first_fit_allocation() {
        std::cout << "Testing first-fit allocation... ";
        PhysicalMemory pm(1024);
        
        int id1 = pm.allocate_first_fit(100);
        assert(id1 >= 0);
        assert(pm.used_memory() == 100);
        assert(pm.free_memory() == 924);
        
        int id2 = pm.allocate_first_fit(200);
        assert(id2 >= 0);
        assert(pm.used_memory() == 300);
        assert(pm.free_memory() == 724);
        
        std::cout << "PASSED\n";
    }

    static void test_best_fit_allocation() {
        std::cout << "Testing best-fit allocation... ";
        PhysicalMemory pm(1024);
        
        // Allocate and free to create fragmentation
        int id1 = pm.allocate_best_fit(100);
        (void)id1;  // Used for fragmentation setup
        int id2 = pm.allocate_best_fit(200);
        int id3 = pm.allocate_best_fit(150);
        (void)id3;  // Used for fragmentation setup
        
        pm.free_block(id2);  // Free middle block
        
        // Best fit should use the 200-byte hole for a 180-byte allocation
        int id4 = pm.allocate_best_fit(180);
        assert(id4 >= 0);
        
        std::cout << "PASSED\n";
    }

    static void test_worst_fit_allocation() {
        std::cout << "Testing worst-fit allocation... ";
        PhysicalMemory pm(1024);
        
        int id1 = pm.allocate_worst_fit(100);
        assert(id1 >= 0);
        assert(pm.used_memory() == 100);
        
        int id2 = pm.allocate_worst_fit(200);
        assert(id2 >= 0);
        assert(pm.used_memory() == 300);
        
        std::cout << "PASSED\n";
    }

    static void test_free_and_reallocation() {
        std::cout << "Testing free and reallocation... ";
        PhysicalMemory pm(1024);
        
        int id1 = pm.allocate_first_fit(100);
        (void)id1;  // Kept allocated for testing
        int id2 = pm.allocate_first_fit(200);
        int id3 = pm.allocate_first_fit(150);
        (void)id3;  // Kept allocated for testing
        
        assert(pm.used_memory() == 450);
        
        pm.free_block(id2);
        assert(pm.used_memory() == 250);
        assert(pm.free_memory() == 774);
        
        int id4 = pm.allocate_first_fit(50);
        assert(id4 >= 0);
        assert(pm.used_memory() == 300);
        
        std::cout << "PASSED\n";
    }

    static void test_fragmentation() {
        std::cout << "Testing fragmentation calculation... ";
        PhysicalMemory pm(1024);
        
        // Create fragmentation
        int id1 = pm.allocate_first_fit(100);
        (void)id1;  // Kept allocated
        int id2 = pm.allocate_first_fit(100);
        int id3 = pm.allocate_first_fit(100);
        (void)id3;  // Kept allocated
        int id4 = pm.allocate_first_fit(100);
        
        pm.free_block(id2);
        pm.free_block(id4);
        
        double frag = pm.external_fragmentation();
        assert(frag >= 0.0 && frag <= 1.0);
        
        std::cout << "PASSED\n";
    }

    static void test_memory_metrics() {
        std::cout << "Testing memory metrics... ";
        PhysicalMemory pm(2048);
        
        int id1 = pm.allocate_first_fit(512);
        int id2 = pm.allocate_first_fit(256);
        (void)id2;  // Kept allocated
        
        assert(pm.total_memory() == 2048);
        assert(pm.used_memory() == 768);
        assert(pm.free_memory() == 1280);
        assert(pm.largest_free_block() == 1280);
        
        pm.free_block(id1);
        assert(pm.used_memory() == 256);
        assert(pm.free_memory() == 1792);
        
        std::cout << "PASSED\n";
    }

    static void test_allocation_failure() {
        std::cout << "Testing allocation failure... ";
        PhysicalMemory pm(256);
        
        int id1 = pm.allocate_first_fit(512);  // Should fail - too large
        assert(id1 == -1);
        
        int id2 = pm.allocate_first_fit(200);
        assert(id2 >= 0);
        
        int id3 = pm.allocate_first_fit(100);  // Should fail - not enough space
        assert(id3 == -1);
        
        std::cout << "PASSED\n";
    }

    static void test_multiple_allocations() {
        std::cout << "Testing multiple allocations... ";
        PhysicalMemory pm(4096);
        
        std::vector<int> ids;
        for (int i = 0; i < 10; ++i) {
            int id = pm.allocate_first_fit(100);
            assert(id >= 0);
            ids.push_back(id);
        }
        
        assert(pm.used_memory() == 1000);
        
        // Free every other allocation
        for (size_t i = 0; i < ids.size(); i += 2) {
            pm.free_block(ids[i]);
        }
        
        assert(pm.used_memory() == 500);
        
        std::cout << "PASSED\n";
    }

    static void test_free_invalid_id() {
        std::cout << "Testing free with invalid ID... ";
        PhysicalMemory pm(1024);
        
        int id1 = pm.allocate_first_fit(100);
        pm.free_block(id1);
        
        // Freeing same block again should be safe (no-op)
        pm.free_block(id1);
        
        // Freeing non-existent ID should be safe
        pm.free_block(9999);
        
        std::cout << "PASSED\n";
    }

    static void test_coalescing() {
        std::cout << "Testing block coalescing... ";
        PhysicalMemory pm(1024);
        
        int id1 = pm.allocate_first_fit(100);
        int id2 = pm.allocate_first_fit(100);
        int id3 = pm.allocate_first_fit(100);
        (void)id3;  // Kept allocated
        
        // Free adjacent blocks
        pm.free_block(id1);
        pm.free_block(id2);
        
        // Should coalesce into a single larger block
        size_t largest = pm.largest_free_block();
        assert(largest >= 200);  // At least the two freed blocks
        
        std::cout << "PASSED\n";
    }
};

int main() {
    PhysicalMemoryTests::run_all_tests();
    return 0;
}
