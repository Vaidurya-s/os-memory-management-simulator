#include "../include/buddy/BuddyAllocator.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>

class BuddyAllocatorTests {
public:
    static void run_all_tests() {
        std::cout << "\n=== Running BuddyAllocator Tests ===\n";
        
        test_initialization();
        test_simple_allocation();
        test_power_of_two_rounding();
        test_allocation_and_free();
        test_buddy_splitting();
        test_buddy_coalescing();
        test_multiple_allocations();
        test_fragmentation_metrics();
        test_allocation_failure();
        test_stress_test();
        test_invariants();
        test_largest_free_block();
        
        std::cout << "=== All BuddyAllocator Tests Passed! ===\n\n";
    }

private:
    static void test_initialization() {
        std::cout << "Testing initialization... ";
        std::cout << "\n  [DEBUG] Creating BuddyAllocator with size 1024 bytes\n";
        BuddyAllocator buddy(1024);
        
        std::cout << "  [EXPECTED] total_memory = 1024\n";
        std::cout << "  [ACTUAL]   total_memory = " << buddy.total_memory() << "\n";
        assert(buddy.total_memory() == 1024);
        
        std::cout << "  [EXPECTED] allocated_memory = 0\n";
        std::cout << "  [ACTUAL]   allocated_memory = " << buddy.allocated_memory() << "\n";
        assert(buddy.allocated_memory() == 0);
        
        std::cout << "  [EXPECTED] free_memory = 1024\n";
        std::cout << "  [ACTUAL]   free_memory = " << buddy.free_memory() << "\n";
        assert(buddy.free_memory() == 1024);
        
        std::cout << "  [EXPECTED] largest_free_block = 1024\n";
        std::cout << "  [ACTUAL]   largest_free_block = " << buddy.largest_free_block() << "\n";
        assert(buddy.largest_free_block() == 1024);
        
        std::cout << "PASSED\n";
    }

    static void test_simple_allocation() {
        std::cout << "Testing simple allocation... ";
        std::cout << "\n  [DEBUG] Creating BuddyAllocator with size 1024 bytes\n";
        BuddyAllocator buddy(1024);
        
        std::cout << "  [STEP 1] Allocating 64 bytes\n";
        std::cout << "  [EXPECTED] Returns valid address (not -1)\n";
        size_t addr1 = buddy.allocate(64);
        std::cout << "  [ACTUAL]   addr1 = 0x" << std::hex << addr1 << std::dec << "\n";
        assert(addr1 != static_cast<size_t>(-1));
        
        std::cout << "  [EXPECTED] allocated_memory > 0\n";
        std::cout << "  [ACTUAL]   allocated_memory = " << buddy.allocated_memory() << "\n";
        assert(buddy.allocated_memory() > 0);
        
        std::cout << "  [STEP 2] Allocating 128 bytes\n";
        std::cout << "  [EXPECTED] Returns different valid address\n";
        size_t addr2 = buddy.allocate(128);
        std::cout << "  [ACTUAL]   addr2 = 0x" << std::hex << addr2 << std::dec << "\n";
        assert(addr2 != static_cast<size_t>(-1));
        
        std::cout << "  [EXPECTED] addr1 != addr2\n";
        std::cout << "  [ACTUAL]   Different addresses: " << (addr1 != addr2 ? "YES" : "NO") << "\n";
        assert(addr1 != addr2);
        
        std::cout << "PASSED\n";
    }

    static void test_power_of_two_rounding() {
        std::cout << "Testing power-of-two rounding... ";
        BuddyAllocator buddy(2048);
        
        // Request 100 bytes, should round up to 128
        size_t addr1 = buddy.allocate(100);
        assert(addr1 != static_cast<size_t>(-1));
        
        // Request 200 bytes, should round up to 256
        size_t addr2 = buddy.allocate(200);
        assert(addr2 != static_cast<size_t>(-1));
        
        std::cout << "PASSED\n";
    }

    static void test_allocation_and_free() {
        std::cout << "Testing allocation and free... ";
        BuddyAllocator buddy(1024);
        
        size_t addr1 = buddy.allocate(64);
        assert(buddy.allocated_memory() >= 64);
        
        size_t initial_allocated = buddy.allocated_memory();
        
        buddy.free_block(addr1);
        assert(buddy.allocated_memory() < initial_allocated);
        assert(buddy.free_memory() == buddy.total_memory());
        
        std::cout << "PASSED\n";
    }

    static void test_buddy_splitting() {
        std::cout << "Testing buddy splitting... ";
        BuddyAllocator buddy(1024);
        
        // First allocation should split larger blocks
        size_t addr1 = buddy.allocate(128);
        assert(addr1 != static_cast<size_t>(-1));
        
        // Second allocation of same size should use buddy
        size_t addr2 = buddy.allocate(128);
        assert(addr2 != static_cast<size_t>(-1));
        assert(addr1 != addr2);
        
        // Verify addresses are aligned and different
        assert((addr1 % 128) == 0);
        assert((addr2 % 128) == 0);
        
        std::cout << "PASSED\n";
    }

    static void test_buddy_coalescing() {
        std::cout << "Testing buddy coalescing... ";
        std::cout << "\n  [DEBUG] Creating 1024-byte buddy system\n";
        BuddyAllocator buddy(1024);
        
        std::cout << "  [STEP 1] Allocate 128 bytes (buddy 1)\n";
        size_t addr1 = buddy.allocate(128);
        std::cout << "  [RESULT]  addr1 = 0x" << std::hex << addr1 << std::dec << "\n";
        std::cout << "  [STATE]   allocated = " << buddy.allocated_memory() << " bytes\n";
        
        std::cout << "  [STEP 2] Allocate 128 bytes (buddy 2)\n";
        size_t addr2 = buddy.allocate(128);
        std::cout << "  [RESULT]  addr2 = 0x" << std::hex << addr2 << std::dec << "\n";
        std::cout << "  [STATE]   allocated = " << buddy.allocated_memory() << " bytes\n";
        
        std::cout << "  [CALC]    These should be buddy pairs\n";
        std::cout << "  [CALC]    Address difference = " << std::abs(static_cast<long>(addr1 - addr2)) << " bytes\n";
        
        std::cout << "  [STEP 3] Free both buddies\n";
        buddy.free_block(addr1);
        std::cout << "  [DEBUG]   Freed addr1, allocated = " << buddy.allocated_memory() << "\n";
        buddy.free_block(addr2);
        std::cout << "  [DEBUG]   Freed addr2, allocated = " << buddy.allocated_memory() << "\n";
        
        std::cout << "  [EXPECTED] After coalescing: free_memory = 1024\n";
        std::cout << "  [ACTUAL]   free_memory = " << buddy.free_memory() << "\n";
        assert(buddy.free_memory() == buddy.total_memory());
        
        std::cout << "  [EXPECTED] After coalescing: largest_free_block = 1024\n";
        std::cout << "  [ACTUAL]   largest_free_block = " << buddy.largest_free_block() << "\n";
        assert(buddy.largest_free_block() == 1024);
        
        std::cout << "PASSED\n";
    }

    static void test_multiple_allocations() {
        std::cout << "Testing multiple allocations... ";
        BuddyAllocator buddy(4096);
        
        std::vector<size_t> addrs;
        
        // Allocate multiple blocks
        for (int i = 0; i < 10; ++i) {
            size_t addr = buddy.allocate(64);
            assert(addr != static_cast<size_t>(-1));
            addrs.push_back(addr);
        }
        
        // All addresses should be unique
        for (size_t i = 0; i < addrs.size(); ++i) {
            for (size_t j = i + 1; j < addrs.size(); ++j) {
                assert(addrs[i] != addrs[j]);
            }
        }
        
        // Free all blocks
        for (size_t addr : addrs) {
            buddy.free_block(addr);
        }
        
        assert(buddy.free_memory() == buddy.total_memory());
        
        std::cout << "PASSED\n";
    }

    static void test_fragmentation_metrics() {
        std::cout << "Testing fragmentation metrics... ";
        BuddyAllocator buddy(2048);
        
        size_t addr1 = buddy.allocate(256);
        size_t addr2 = buddy.allocate(128);
        
        double frag = buddy.internal_fragmentation();
        assert(frag >= 0.0 && frag <= 1.0);
        
        buddy.free_block(addr1);
        buddy.free_block(addr2);
        
        // After freeing all, fragmentation should be 0
        frag = buddy.internal_fragmentation();
        assert(frag == 0.0);
        
        std::cout << "PASSED\n";
    }

    static void test_allocation_failure() {
        std::cout << "Testing allocation failure... ";
        BuddyAllocator buddy(512);
        
        // Try to allocate more than available
        size_t addr = buddy.allocate(1024);
        assert(addr == static_cast<size_t>(-1));
        
        // Fill up memory
        size_t addr1 = buddy.allocate(256);
        size_t addr2 = buddy.allocate(256);
        
        // No more space left
        size_t addr3 = buddy.allocate(256);
        assert(addr3 == static_cast<size_t>(-1));
        
        std::cout << "PASSED\n";

        buddy.free_block(addr1);
        buddy.free_block(addr2);
    }

    static void test_stress_test() {
        std::cout << "Testing stress scenario... ";
        BuddyAllocator buddy(8192);
        
        std::vector<size_t> addrs;
        
        // Allocate many different sizes
        for (int i = 0; i < 20; ++i) {
            size_t size = 32 * (1 << (i % 4));  // 32, 64, 128, 256
            size_t addr = buddy.allocate(size);
            if (addr != static_cast<size_t>(-1)) {
                addrs.push_back(addr);
            }
        }
        
        // Free every other allocation
        for (size_t i = 0; i < addrs.size(); i += 2) {
            buddy.free_block(addrs[i]);
        }
        
        // Allocate again
        for (int i = 0; i < 5; ++i) {
            buddy.allocate(64);
        }
        
        // Clean up
        for (size_t i = 1; i < addrs.size(); i += 2) {
            buddy.free_block(addrs[i]);
        }
        
        std::cout << "PASSED\n";
    }

    static void test_invariants() {
        std::cout << "Testing invariants... ";
        BuddyAllocator buddy(2048);
        
        size_t addr1 = buddy.allocate(128);
        size_t addr2 = buddy.allocate(256);
        
        // Check invariants
        assert(buddy.check_no_overlaps());
        assert(buddy.check_no_free_buddy_pairs());
        
        buddy.free_block(addr1);
        
        assert(buddy.check_no_overlaps());
        
        buddy.free_block(addr2);
        
        assert(buddy.check_no_overlaps());
        assert(buddy.check_no_free_buddy_pairs());
        
        std::cout << "PASSED\n";
    }

    static void test_largest_free_block() {
        std::cout << "Testing largest free block tracking... ";
        BuddyAllocator buddy(2048);
        
        assert(buddy.largest_free_block() == 2048);
        
        size_t addr1 = buddy.allocate(512);
        assert(buddy.largest_free_block() == 1024);
        
        size_t addr2 = buddy.allocate(1024);
        assert(buddy.largest_free_block() == 512);
        
        buddy.free_block(addr2);
        assert(buddy.largest_free_block() == 1024);
        
        buddy.free_block(addr1);
        assert(buddy.largest_free_block() == 2048);
        
        std::cout << "PASSED\n";
    }
};

int main() {
    BuddyAllocatorTests::run_all_tests();
    return 0;
}
