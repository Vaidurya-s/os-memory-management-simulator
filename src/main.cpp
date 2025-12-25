#include "virtual_memory/VirtualMemoryManager.h"
#include "cache/CacheHierarchy.h"

#include <iostream>

int main() {
    // Virtual memory: very small to force replacement
    VirtualMemoryManager vmm(
        4,      // virtual pages
        2,      // physical frames
        4096    // page size
    );

    // Cache hierarchy
    DirectMappedCache l1(256, 64, 1);
    DirectMappedCache l2(1024, 64, 2);
    CacheHierarchy cache(l1, l2);

    // Virtual access pattern
    std::uint64_t vaddrs[] = {
        0x0000, // page 0
        0x1000, // page 1
        0x2000, // page 2 (evicts page 0)
        0x1000, // page 1 (hit)
        0x0000  // page 0 (fault again)
    };

    for (std::uint64_t va : vaddrs) {
        std::uint64_t pa = vmm.translate(va);
        cache.access(pa);
    }

    std::cout << "Page faults: " << vmm.page_faults() << "\n";

    std::cout << "L1 hits: " << cache.l1_hits() << "\n";
    std::cout << "L1 misses: " << cache.l1_misses() << "\n";

    std::cout << "L2 hits: " << cache.l2_hits() << "\n";
    std::cout << "L2 misses: " << cache.l2_misses() << "\n";

    return 0;
}

// #include <iostream>
// #include <vector>
// #include <cstdint>

// #include "virtual_memory/VirtualMemoryManager.h"

// int main() {
//     constexpr std::size_t page_size = 4096;
//     constexpr std::size_t num_virtual_pages = 8;
//     constexpr std::size_t num_frames = 3;

//     // Canonical reference string
//     std::vector<std::uint64_t> access_pattern = {
//         1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5
//     };

//     auto run_test = [&](VirtualMemoryManager::PageReplacementPolicy policy,
//                         const std::string& name) {
//         VirtualMemoryManager vmm(
//             num_virtual_pages,
//             num_frames,
//             page_size,
//             policy
//         );

//         // ------------------------
//         // Warm-up phase
//         // ------------------------
//         // Run the pattern multiple times to fill frames
//         // and stabilize recency information
//         for (int round = 0; round < 5; ++round) {
//             for (std::uint64_t vpn : access_pattern) {
//                 std::uint64_t vaddr = vpn * page_size;
//                 vmm.translate(vaddr);
//             }
//         }

//         // Record faults after warm-up
//         std::size_t faults_after_warmup = vmm.page_faults();

//         // ------------------------
//         // Measurement phase
//         // ------------------------
//         for (std::uint64_t vpn : access_pattern) {
//             std::uint64_t vaddr = vpn * page_size;
//             vmm.translate(vaddr);
//         }

//         std::size_t total_faults = vmm.page_faults();
//         std::size_t steady_state_faults =
//             total_faults - faults_after_warmup;

//         std::cout << name << " results\n";
//         std::cout << "Warm-up faults: "
//                   << faults_after_warmup << "\n";
//         std::cout << "Steady-state faults: "
//                   << steady_state_faults << "\n\n";
//     };

//     std::cout << "Running Virtual Memory Replacement Policy Warm-Up Test\n\n";

//     run_test(VirtualMemoryManager::PageReplacementPolicy::FIFO, "FIFO");
//     run_test(VirtualMemoryManager::PageReplacementPolicy::LRU,  "LRU");

//     return 0;
// }
