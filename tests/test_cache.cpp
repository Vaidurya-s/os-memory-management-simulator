#include "../include/cache/DirectMappedCache.h"
#include <iostream>
#include <cassert>
#include <vector>

class DirectMappedCacheTests {
public:
    static void run_all_tests() {
        std::cout << "\n=== Running DirectMappedCache Tests ===\n";
        
        test_initialization();
        test_address_decoding();
        test_cache_miss();
        test_cache_hit();
        test_cache_replacement();
        test_hit_ratio();
        test_sequential_access();
        test_strided_access();
        test_associativity();
        test_conflict_misses();
        test_cache_size_variations();
        test_line_size_variations();
        
        std::cout << "=== All DirectMappedCache Tests Passed! ===\n\n";
    }

private:
    static void test_initialization() {
        std::cout << "Testing initialization... ";
        DirectMappedCache cache(4096, 64, 1);
        
        assert(cache.hits() == 0);
        assert(cache.misses() == 0);
        assert(cache.hit_ratio() == 0.0);
        assert(cache.num_sets() > 0);
        
        std::cout << "PASSED\n";
    }

    static void test_address_decoding() {
        std::cout << "Testing address decoding... ";
        DirectMappedCache cache(1024, 64, 1);  // 16 sets, 6-bit offset
        
        uint64_t addr = 0x1234;
        CacheAddress decoded = cache.decode_address(addr);
        
        // Verify offset is within line size
        assert(decoded.offset < 64);
        // Verify index is within number of sets
        assert(decoded.index < cache.num_sets());
        
        std::cout << "PASSED\n";
    }

    static void test_cache_miss() {
        std::cout << "Testing cache miss... ";
        DirectMappedCache cache(1024, 64, 1);
        
        bool hit = cache.access(0x1000);
        assert(!hit);
        assert(cache.misses() == 1);
        assert(cache.hits() == 0);
        
        std::cout << "PASSED\n";
    }

    static void test_cache_hit() {
        std::cout << "Testing cache hit... ";
        DirectMappedCache cache(1024, 64, 1);
        
        uint64_t addr = 0x1000;
        
        // First access - miss
        bool hit1 = cache.access(addr);
        assert(!hit1);
        cache.fill(addr);
        
        // Second access - should be hit
        bool hit2 = cache.access(addr);
        assert(hit2);
        assert(cache.hits() == 1);
        assert(cache.misses() == 1);
        
        std::cout << "PASSED\n";
    }

    static void test_cache_replacement() {
        std::cout << "Testing cache replacement... ";
        DirectMappedCache cache(1024, 64, 1);  // 16 sets
        
        uint64_t addr1 = 0x0000;
        uint64_t addr2 = 0x0400;  // Maps to same set (1024 bytes apart)
        
        // Access first address
        cache.access(addr1);
        cache.fill(addr1);
        assert(cache.access(addr1));  // Hit
        
        // Access conflicting address - should evict first
        cache.access(addr2);
        cache.fill(addr2);
        
        // First address should now miss
        bool hit = cache.access(addr1);
        assert(!hit);
        
        std::cout << "PASSED\n";
    }

    static void test_hit_ratio() {
        std::cout << "Testing hit ratio calculation... ";
        DirectMappedCache cache(2048, 64, 1);
        
        // 5 misses, 3 hits
        for (int i = 0; i < 8; ++i) {
            uint64_t addr = i * 64;
            bool hit = cache.access(addr);
            if (!hit) {
                cache.fill(addr);
            }
            
            if (i >= 5) {
                // Re-access first 3 addresses for hits
                cache.access((i - 5) * 64);
            }
        }
        
        double ratio = cache.hit_ratio();
        assert(ratio >= 0.0 && ratio <= 1.0);
        
        std::cout << "PASSED\n";
    }

    static void test_sequential_access() {
        std::cout << "Testing sequential access pattern... ";
        DirectMappedCache cache(1024, 64, 1);
        
        // Sequential access - first pass all misses
        for (int i = 0; i < 16; ++i) {
            uint64_t addr = i * 64;
            bool hit = cache.access(addr);
            if (!hit) {
                cache.fill(addr);
            }
        }
        
        size_t first_misses = cache.misses();
        assert(first_misses > 0);
        
        // Second pass - should have some hits
        for (int i = 0; i < 16; ++i) {
            uint64_t addr = i * 64;
            cache.access(addr);
        }
        
        assert(cache.hits() > 0);
        
        std::cout << "PASSED\n";
    }

    static void test_strided_access() {
        std::cout << "Testing strided access pattern... ";
        DirectMappedCache cache(2048, 64, 1);
        
        // Access with large stride
        size_t stride = 256;
        for (int i = 0; i < 10; ++i) {
            uint64_t addr = i * stride;
            bool hit = cache.access(addr);
            if (!hit) {
                cache.fill(addr);
            }
        }
        
        assert(cache.misses() > 0);
        
        std::cout << "PASSED\n";
    }

    static void test_associativity() {
        std::cout << "Testing different associativity... ";
        
        // Direct-mapped (1-way)
        DirectMappedCache cache1(1024, 64, 1);
        assert(cache1.num_sets() == 16);
        
        // 2-way set-associative
        DirectMappedCache cache2(1024, 64, 2);
        assert(cache2.num_sets() == 8);
        
        // 4-way set-associative
        DirectMappedCache cache4(1024, 64, 4);
        assert(cache4.num_sets() == 4);
        
        std::cout << "PASSED\n";
    }

    static void test_conflict_misses() {
        std::cout << "Testing conflict misses... ";
        DirectMappedCache cache(512, 64, 1);  // 8 sets
        
        // Access addresses that map to the same set
        std::vector<uint64_t> conflicting_addrs = {
            0x0000,
            0x0200,  // 512 bytes apart - same set
            0x0400,  // 1024 bytes apart - same set
        };
        
        for (uint64_t addr : conflicting_addrs) {
            cache.access(addr);
            cache.fill(addr);
        }
        
        // All should be misses in direct-mapped
        assert(cache.misses() == conflicting_addrs.size());
        
        // Re-accessing first address should miss (evicted)
        bool hit = cache.access(0x0000);
        assert(!hit);
        
        std::cout << "PASSED\n";
    }

    static void test_cache_size_variations() {
        std::cout << "Testing various cache sizes... ";
        
        std::vector<size_t> sizes = {512, 1024, 2048, 4096, 8192};
        
        for (size_t size : sizes) {
            DirectMappedCache cache(size, 64, 1);
            assert(cache.num_sets() == size / 64);
            
            // Basic access test
            cache.access(0x1000);
            cache.fill(0x1000);
            bool hit = cache.access(0x1000);
            assert(hit);
        }
        
        std::cout << "PASSED\n";
    }

    static void test_line_size_variations() {
        std::cout << "Testing various line sizes... ";
        
        std::vector<size_t> line_sizes = {16, 32, 64, 128};
        
        for (size_t line_size : line_sizes) {
            DirectMappedCache cache(1024, line_size, 1);
            assert(cache.num_sets() == 1024 / line_size);
            
            // Test spatial locality with different line sizes
            uint64_t base_addr = 0x1000;
            cache.access(base_addr);
            cache.fill(base_addr);
            
            // Access within same cache line should hit
            bool hit = cache.access(base_addr + line_size / 2);
            assert(hit);
        }
        
        std::cout << "PASSED\n";
    }
};

int main() {
    DirectMappedCacheTests::run_all_tests();
    return 0;
}
