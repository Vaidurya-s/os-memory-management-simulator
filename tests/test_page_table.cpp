#include "../include/virtual_memory/PageTable.h"
#include <iostream>
#include <cassert>

class PageTableTests {
public:
    static void run_all_tests() {
        std::cout << "\n=== Running PageTable Tests ===\n";
        
        test_initialization();
        test_entry_access();
        test_entry_modification();
        test_valid_bit();
        test_frame_number();
        test_timestamp();
        test_dirty_bit();
        test_referenced_bit();
        test_multiple_entries();
        test_boundary_conditions();
        
        std::cout << "=== All PageTable Tests Passed! ===\n\n";
    }

private:
    static void test_initialization() {
        std::cout << "Testing initialization... ";
        PageTable pt(64);
        
        assert(pt.size() == 64);
        
        // All entries should be invalid initially
        for (size_t i = 0; i < 64; ++i) {
            assert(!pt.entry(i).valid);
        }
        
        std::cout << "PASSED\n";
    }

    static void test_entry_access() {
        std::cout << "Testing entry access... ";
        PageTable pt(32);
        
        // Access entries
        PageTableEntry& entry0 = pt.entry(0);
        PageTableEntry& entry10 = pt.entry(10);
        PageTableEntry& entry31 = pt.entry(31);
        
        // Modify entries
        entry0.valid = true;
        entry10.frame_number = 5;
        entry31.loaded_at = 1000;
        
        // Verify modifications
        assert(pt.entry(0).valid == true);
        assert(pt.entry(10).frame_number == 5);
        assert(pt.entry(31).loaded_at == 1000);
        
        std::cout << "PASSED\n";
    }

    static void test_entry_modification() {
        std::cout << "Testing entry modification... ";
        PageTable pt(16);
        
        PageTableEntry& entry = pt.entry(5);
        
        // Initial state
        assert(!entry.valid);
        assert(entry.frame_number == 0);
        
        // Modify
        entry.valid = true;
        entry.frame_number = 10;
        entry.loaded_at = 500;
        
        // Verify
        assert(entry.valid == true);
        assert(entry.frame_number == 10);
        assert(entry.loaded_at == 500);
        
        std::cout << "PASSED\n";
    }

    static void test_valid_bit() {
        std::cout << "Testing valid bit... ";
        PageTable pt(8);
        
        // Set valid bits
        pt.entry(0).valid = true;
        pt.entry(3).valid = true;
        pt.entry(7).valid = true;
        
        // Verify
        assert(pt.entry(0).valid);
        assert(!pt.entry(1).valid);
        assert(!pt.entry(2).valid);
        assert(pt.entry(3).valid);
        assert(!pt.entry(4).valid);
        assert(pt.entry(7).valid);
        
        std::cout << "PASSED\n";
    }

    static void test_frame_number() {
        std::cout << "Testing frame number assignment... ";
        PageTable pt(16);
        
        // Assign different frame numbers
        for (size_t i = 0; i < 10; ++i) {
            pt.entry(i).frame_number = i * 2;
        }
        
        // Verify
        for (size_t i = 0; i < 10; ++i) {
            assert(pt.entry(i).frame_number == i * 2);
        }
        
        std::cout << "PASSED\n";
    }

    static void test_timestamp() {
        std::cout << "Testing timestamp tracking... ";
        PageTable pt(32);
        
        // Set timestamps
        pt.entry(0).loaded_at = 100;
        pt.entry(5).loaded_at = 200;
        pt.entry(10).loaded_at = 150;
        
        // Verify ordering
        assert(pt.entry(0).loaded_at < pt.entry(5).loaded_at);
        assert(pt.entry(10).loaded_at > pt.entry(0).loaded_at);
        assert(pt.entry(10).loaded_at < pt.entry(5).loaded_at);
        
        std::cout << "PASSED\n";
    }

    static void test_dirty_bit() {
        std::cout << "Testing dirty bit... ";
        PageTable pt(16);
        
        // Initially not dirty
        assert(!pt.entry(0).dirty);
        
        // Mark as dirty
        pt.entry(0).dirty = true;
        assert(pt.entry(0).dirty);
        
        // Clear dirty bit
        pt.entry(0).dirty = false;
        assert(!pt.entry(0).dirty);
        
        std::cout << "PASSED\n";
    }

    static void test_referenced_bit() {
        std::cout << "Testing referenced bit... ";
        PageTable pt(16);
        
        // Initially not referenced
        assert(!pt.entry(0).referenced);
        
        // Mark as referenced
        pt.entry(0).referenced = true;
        assert(pt.entry(0).referenced);
        
        // Clear referenced bit
        pt.entry(0).referenced = false;
        assert(!pt.entry(0).referenced);
        
        std::cout << "PASSED\n";
    }

    static void test_multiple_entries() {
        std::cout << "Testing multiple entries... ";
        PageTable pt(64);
        
        // Set up multiple entries
        for (size_t i = 0; i < 32; ++i) {
            pt.entry(i).valid = (i % 2 == 0);
            pt.entry(i).frame_number = i;
            pt.entry(i).loaded_at = i * 100;
            pt.entry(i).dirty = (i % 3 == 0);
            pt.entry(i).referenced = (i % 5 == 0);
        }
        
        // Verify all entries
        for (size_t i = 0; i < 32; ++i) {
            assert(pt.entry(i).valid == (i % 2 == 0));
            assert(pt.entry(i).frame_number == i);
            assert(pt.entry(i).loaded_at == i * 100);
            assert(pt.entry(i).dirty == (i % 3 == 0));
            assert(pt.entry(i).referenced == (i % 5 == 0));
        }
        
        std::cout << "PASSED\n";
    }

    static void test_boundary_conditions() {
        std::cout << "Testing boundary conditions... ";
        PageTable pt(128);
        
        // Test first entry
        pt.entry(0).valid = true;
        pt.entry(0).frame_number = 999;
        assert(pt.entry(0).valid);
        assert(pt.entry(0).frame_number == 999);
        
        // Test last entry
        pt.entry(127).valid = true;
        pt.entry(127).frame_number = 888;
        assert(pt.entry(127).valid);
        assert(pt.entry(127).frame_number == 888);
        
        // Test middle entry
        pt.entry(64).valid = true;
        pt.entry(64).frame_number = 777;
        assert(pt.entry(64).valid);
        assert(pt.entry(64).frame_number == 777);
        
        std::cout << "PASSED\n";
    }
};

int main() {
    PageTableTests::run_all_tests();
    return 0;
}
