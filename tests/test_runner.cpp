#include <iostream>
#include <cstdlib>
#include <string>

// Test runner declarations
int run_physical_memory_tests();
int run_buddy_allocator_tests();
int run_cache_tests();
int run_virtual_memory_tests();
int run_page_table_tests();
int run_virtual_address_tests();

void print_header() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "   Memory Simulator - Test Suite       \n";
    std::cout << "========================================\n";
}

void print_summary(int total, int passed, int failed) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "           Test Summary                 \n";
    std::cout << "========================================\n";
    std::cout << "Total test suites:  " << total << "\n";
    std::cout << "Passed:             " << passed << "\n";
    std::cout << "Failed:             " << failed << "\n";
    
    if (failed == 0) {
        std::cout << "\n✓ ALL TESTS PASSED!\n";
    } else {
        std::cout << "\n✗ SOME TESTS FAILED!\n";
    }
    std::cout << "========================================\n\n";
}

int run_all_tests() {
    
    print_header();
    
    std::cout << "\nRunning comprehensive test suite...\n";
    std::cout << "====================================\n";
    
    // Note: In a real implementation, each test suite would be in its own executable
    // and this would run them. For now, this is a template.
    
    std::cout << "\nTo run individual test suites, compile and run:\n";
    std::cout << "  - test_physical_memory\n";
    std::cout << "  - test_buddy_allocator\n";
    std::cout << "  - test_cache\n";
    std::cout << "  - test_virtual_memory\n";
    std::cout << "  - test_page_table\n";
    std::cout << "  - test_virtual_address\n";
    
    return 0;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --all                Run all test suites\n";
    std::cout << "  --physical-memory    Run PhysicalMemory tests\n";
    std::cout << "  --buddy              Run BuddyAllocator tests\n";
    std::cout << "  --cache              Run DirectMappedCache tests\n";
    std::cout << "  --virtual-memory     Run VirtualMemoryManager tests\n";
    std::cout << "  --page-table         Run PageTable tests\n";
    std::cout << "  --virtual-address    Run VirtualAddressDecoder tests\n";
    std::cout << "  --help               Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " --all\n";
    std::cout << "  " << program_name << " --cache --virtual-memory\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Memory Simulator Test Runner\n";
        std::cout << "Use --help for usage information\n";
        std::cout << "\nRunning all tests by default...\n";
        return run_all_tests();
    }
    
    bool run_all = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--all") {
            run_all = true;
        }
    }
    
    if (run_all) {
        return run_all_tests();
    }
    
    std::cout << "No valid test options specified.\n";
    std::cout << "Use --help for usage information\n";
    return 1;
}
