#include "../include/virtual_memory/VirtualAddress.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>

class VirtualAddressDecoderTests {
public:
    static void run_all_tests() {
        std::cout << "\n=== Running VirtualAddressDecoder Tests ===\n";
        
        test_initialization();
        test_decode_simple();
        test_decode_page_sizes();
        test_vpn_extraction();
        test_offset_extraction();
        test_boundary_addresses();
        test_all_zeros();
        test_all_ones();
        test_sequential_addresses();
        test_various_page_sizes();
        
        std::cout << "=== All VirtualAddressDecoder Tests Passed! ===\n\n";
    }

private:
    static void test_initialization() {
        std::cout << "Testing initialization... ";
        
        VirtualAddressDecoder decoder(4096);  // 4KB pages
        
        // Decoder should be ready to use
        VirtualAddress va = decoder.decode(0);
        assert(va.vpn == 0);
        assert(va.offset == 0);
        
        std::cout << "PASSED\n";
    }

    static void test_decode_simple() {
        std::cout << "Testing simple decode... ";
        VirtualAddressDecoder decoder(4096);  // 4KB pages (12-bit offset)
        
        // Address 0x0000
        VirtualAddress va1 = decoder.decode(0x0000);
        assert(va1.vpn == 0);
        assert(va1.offset == 0);
        
        // Address 0x1234 (page 1, offset 0x234)
        VirtualAddress va2 = decoder.decode(0x1234);
        assert(va2.vpn == 1);
        assert(va2.offset == 0x234);
        
        std::cout << "PASSED\n";
    }

    static void test_decode_page_sizes() {
        std::cout << "Testing different page sizes... ";
        
        // 4KB pages (12-bit offset)
        VirtualAddressDecoder decoder1(4096);
        VirtualAddress va1 = decoder1.decode(0x2ABC);
        assert(va1.vpn == 2);
        assert(va1.offset == 0xABC);
        
        // 8KB pages (13-bit offset)
        VirtualAddressDecoder decoder2(8192);
        VirtualAddress va2 = decoder2.decode(0x3456);
        assert(va2.vpn == 1);
        assert(va2.offset == 0x1456);
        
        // 2KB pages (11-bit offset)
        VirtualAddressDecoder decoder3(2048);
        VirtualAddress va3 = decoder3.decode(0x1234);
        assert(va3.vpn == 2);
        assert(va3.offset == 0x234);
        
        std::cout << "PASSED\n";
    }

    static void test_vpn_extraction() {
        std::cout << "Testing VPN extraction... ";
        VirtualAddressDecoder decoder(4096);
        
        // Test various VPNs
        assert(decoder.decode(0x0000).vpn == 0);
        assert(decoder.decode(0x1000).vpn == 1);
        assert(decoder.decode(0x2000).vpn == 2);
        assert(decoder.decode(0x5000).vpn == 5);
        assert(decoder.decode(0xA000).vpn == 10);
        assert(decoder.decode(0xFF000).vpn == 255);
        
        std::cout << "PASSED\n";
    }

    static void test_offset_extraction() {
        std::cout << "Testing offset extraction... ";
        VirtualAddressDecoder decoder(4096);  // 12-bit offset
        
        // Test various offsets within page 0
        assert(decoder.decode(0x000).offset == 0x000);
        assert(decoder.decode(0x001).offset == 0x001);
        assert(decoder.decode(0x100).offset == 0x100);
        assert(decoder.decode(0x7FF).offset == 0x7FF);
        assert(decoder.decode(0xFFF).offset == 0xFFF);
        
        // Test offsets in other pages
        assert(decoder.decode(0x1234).offset == 0x234);
        assert(decoder.decode(0x2ABC).offset == 0xABC);
        assert(decoder.decode(0x3FFF).offset == 0xFFF);
        
        std::cout << "PASSED\n";
    }

    static void test_boundary_addresses() {
        std::cout << "Testing boundary addresses... ";
        VirtualAddressDecoder decoder(4096);
        
        // First address of page 0
        VirtualAddress va1 = decoder.decode(0x0000);
        assert(va1.vpn == 0);
        assert(va1.offset == 0);
        
        // Last address of page 0
        VirtualAddress va2 = decoder.decode(0x0FFF);
        assert(va2.vpn == 0);
        assert(va2.offset == 0xFFF);
        
        // First address of page 1
        VirtualAddress va3 = decoder.decode(0x1000);
        assert(va3.vpn == 1);
        assert(va3.offset == 0);
        
        // Last address of page 1
        VirtualAddress va4 = decoder.decode(0x1FFF);
        assert(va4.vpn == 1);
        assert(va4.offset == 0xFFF);
        
        std::cout << "PASSED\n";
    }

    static void test_all_zeros() {
        std::cout << "Testing all zeros address... ";
        VirtualAddressDecoder decoder(4096);
        
        VirtualAddress va = decoder.decode(0);
        assert(va.vpn == 0);
        assert(va.offset == 0);
        
        std::cout << "PASSED\n";
    }

    static void test_all_ones() {
        std::cout << "Testing high addresses... ";
        VirtualAddressDecoder decoder(4096);
        
        // High address
        uint64_t addr = 0xFFFFFFFF;
        VirtualAddress va = decoder.decode(addr);
        
        // VPN should be high, offset should be 0xFFF
        assert(va.offset == 0xFFF);
        assert(va.vpn == addr >> 12);
        
        std::cout << "PASSED\n";
    }

    static void test_sequential_addresses() {
        std::cout << "Testing sequential addresses... ";
        VirtualAddressDecoder decoder(4096);
        
        // Sequential addresses within same page
        for (uint64_t i = 0; i < 4096; ++i) {
            VirtualAddress va = decoder.decode(i);
            assert(va.vpn == 0);
            assert(va.offset == i);
        }
        
        // First address of next page
        VirtualAddress va = decoder.decode(4096);
        assert(va.vpn == 1);
        assert(va.offset == 0);
        
        std::cout << "PASSED\n";
    }

    static void test_various_page_sizes() {
        std::cout << "Testing various page sizes... ";
        
        std::vector<size_t> page_sizes = {512, 1024, 2048, 4096, 8192, 16384};
        
        for (size_t page_size : page_sizes) {
            VirtualAddressDecoder decoder(page_size);
            
            size_t offset_bits = static_cast<size_t>(log2(page_size));
            size_t offset_mask = page_size - 1;
            
            // Test address decomposition
            uint64_t test_addr = 0x12345;
            VirtualAddress va = decoder.decode(test_addr);
            
            // Verify offset matches expected bits
            assert(va.offset == (test_addr & offset_mask));
            
            // Verify VPN is correct
            assert(va.vpn == (test_addr >> offset_bits));
            
            // Verify reconstruction
            uint64_t reconstructed = (va.vpn << offset_bits) | va.offset;
            assert(reconstructed == test_addr);
        }
        
        std::cout << "PASSED\n";
    }
};

int main() {
    VirtualAddressDecoderTests::run_all_tests();
    return 0;
}
