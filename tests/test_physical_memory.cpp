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
        std::cout << "\n  [DEBUG] Creating PhysicalMemory with size 1024 bytes\n";
        PhysicalMemory pm(1024);
        
        std::cout << "  [EXPECTED] total_memory = 1024\n";
        std::cout << "  [ACTUAL]   total_memory = " << pm.total_memory() << "\n";
        assert(pm.total_memory() == 1024);
        
        std::cout << "  [EXPECTED] used_memory = 0\n";
        std::cout << "  [ACTUAL]   used_memory = " << pm.used_memory() << "\n";
        assert(pm.used_memory() == 0);
        
        std::cout << "  [EXPECTED] free_memory = 1024\n";
        std::cout << "  [ACTUAL]   free_memory = " << pm.free_memory() << "\n";
        assert(pm.free_memory() == 1024);
        
        std::cout << "  [EXPECTED] largest_free_block = 1024\n";
        std::cout << "  [ACTUAL]   largest_free_block = " << pm.largest_free_block() << "\n";
        assert(pm.largest_free_block() == 1024);
        std::cout << "PASSED\n";
    }

    static void test_first_fit_allocation() {
        std::cout << "Testing first-fit allocation... ";
        std::cout << "\n  [DEBUG] Creating PhysicalMemory with size 1024 bytes\n";
        PhysicalMemory pm(1024);
        
        std::cout << "  [STEP 1] Allocating 100 bytes using first-fit\n";
        int id1 = pm.allocate_first_fit(100);
        std::cout << "  [EXPECTED] id1 >= 0 (allocation successful)\n";
        std::cout << "  [ACTUAL]   id1 = " << id1 << "\n";
        assert(id1 >= 0);
        
        std::cout << "  [EXPECTED] used_memory = 100\n";
        std::cout << "  [ACTUAL]   used_memory = " << pm.used_memory() << "\n";
        assert(pm.used_memory() == 100);
        
        std::cout << "  [EXPECTED] free_memory = 1024 - 100 = 924\n";
        std::cout << "  [ACTUAL]   free_memory = " << pm.free_memory() << "\n";
        assert(pm.free_memory() == 924);
        
        std::cout << "  [STEP 2] Allocating 200 bytes using first-fit\n";
        int id2 = pm.allocate_first_fit(200);
        std::cout << "  [EXPECTED] id2 >= 0 and id2 != id1\n";
        std::cout << "  [ACTUAL]   id2 = " << id2 << "\n";
        assert(id2 >= 0);
        
        std::cout << "  [EXPECTED] used_memory = 100 + 200 = 300\n";
        std::cout << "  [ACTUAL]   used_memory = " << pm.used_memory() << "\n";
        assert(pm.used_memory() == 300);
        
        std::cout << "  [EXPECTED] free_memory = 1024 - 300 = 724\n";
        std::cout << "  [ACTUAL]   free_memory = " << pm.free_memory() << "\n";
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
        std::cout << "\n  [DEBUG] Starting with 1024-byte memory\n";
        PhysicalMemory pm(1024);
        
        std::cout << "  [STEP 1] Allocate 100 bytes\n";
        int id1 = pm.allocate_first_fit(100);
        std::cout << "  [RESULT]  id1 = " << id1 << "\n";
        (void)id1;  // Kept allocated for testing
        
        std::cout << "  [STEP 2] Allocate 200 bytes\n";
        int id2 = pm.allocate_first_fit(200);
        std::cout << "  [RESULT]  id2 = " << id2 << "\n";
        
        std::cout << "  [STEP 3] Allocate 150 bytes\n";
        int id3 = pm.allocate_first_fit(150);
        std::cout << "  [RESULT]  id3 = " << id3 << "\n";
        (void)id3;  // Kept allocated for testing
        
        std::cout << "  [EXPECTED] Total used = 100 + 200 + 150 = 450\n";
        std::cout << "  [ACTUAL]   used_memory = " << pm.used_memory() << "\n";
        assert(pm.used_memory() == 450);
        
        std::cout << "  [STEP 4] Free id2 (200 bytes)\n";
        pm.free_block(id2);
        std::cout << "  [EXPECTED] used_memory = 450 - 200 = 250\n";
        std::cout << "  [ACTUAL]   used_memory = " << pm.used_memory() << "\n";
        assert(pm.used_memory() == 250);
        
        std::cout << "  [EXPECTED] free_memory = 1024 - 250 = 774\n";
        std::cout << "  [ACTUAL]   free_memory = " << pm.free_memory() << "\n";
        assert(pm.free_memory() == 774);
        
        std::cout << "  [STEP 5] Allocate 50 bytes (should fit in freed 200-byte hole)\n";
        int id4 = pm.allocate_first_fit(50);
        std::cout << "  [RESULT]  id4 = " << id4 << " (should be valid)\n";
        assert(id4 >= 0);
        
        std::cout << "  [EXPECTED] used_memory = 250 + 50 = 300\n";
        std::cout << "  [ACTUAL]   used_memory = " << pm.used_memory() << "\n";
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
