#include "allocator/IAllocator.h"
#include "allocator/PhysicalMemory.h"
#include "buddy/BuddyAllocator.h"
#include "cache/CacheHierarchy.h"
#include "cache/DirectMappedCache.h"
#include "virtual_memory/VirtualMemoryManager.h"

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>

class MemorySimulatorCLI {
private:
    IAllocator* allocator;
    std::map<int, size_t> blockSizes;  // Track sizes for display
    
    // Cache hierarchy components
    DirectMappedCache* l1Cache;
    DirectMappedCache* l2Cache;
    CacheHierarchy* cacheHierarchy;
    
    // Virtual memory components
    VirtualMemoryManager* vmManager;
    
    // Integration flags
    bool enableCache;
    bool enableVirtualMemory;
    size_t memorySize;
    
public:
    MemorySimulatorCLI() 
        : allocator(nullptr), 
          l1Cache(nullptr), 
          l2Cache(nullptr), 
          cacheHierarchy(nullptr),
          vmManager(nullptr),
          enableCache(false),
          enableVirtualMemory(false),
          memorySize(0) {}
    
    ~MemorySimulatorCLI() {
        delete allocator;
        delete cacheHierarchy;
        delete l1Cache;
        delete l2Cache;
        delete vmManager;
    }
    
    void run() {
        std::cout << "=================================\n";
        std::cout << "   Memory Simulator CLI\n";
        std::cout << "=================================\n\n";
        
        // Choose allocator at startup
        if (!selectAllocator()) {
            return;
        }
        
        // Ask about optional features
        askOptionalFeatures();
        
        std::cout << "\nMemory simulator ready!\n";
        if (enableVirtualMemory && enableCache) {
            std::cout << "Flow: Virtual Address --> Page Table --> Physical Address --> Cache --> Memory\n";
        } else if (enableVirtualMemory) {
            std::cout << "Flow: Virtual Address --> Page Table --> Physical Address --> Memory\n";
        } else if (enableCache) {
            std::cout << "Flow: Physical Address --> Cache --> Memory\n";
        }
        std::cout << "Type 'help' for available commands\n\n";
        
        std::string line;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, line)) break;
            
            if (line.empty()) continue;
            
            if (line == "exit" || line == "quit") {
                std::cout << "Exiting simulator...\n";
                break;
            }
            
            try {
                processCommand(line);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
            }
        }
    }
    
private:
    void askOptionalFeatures() {
        std::cout << "\n--- Optional Features ---\n";
        std::cout << "Enable cache hierarchy? (Y/N): ";
        std::string response;
        std::getline(std::cin, response);
        
        if (response == "Y" || response == "y") {
            enableCache = true;
            setupCache();
        }
        
        std::cout << "Enable virtual memory? (Y/N): ";
        std::getline(std::cin, response);
        
        if (response == "Y" || response == "y") {
            enableVirtualMemory = true;
            setupVirtualMemory();
        }
    }
    
    void setupCache() {
        // L1: 32KB, 64-byte lines, direct-mapped
        // L2: 256KB, 64-byte lines, direct-mapped
        l1Cache = new DirectMappedCache(32 * 1024, 64, 1);
        l2Cache = new DirectMappedCache(256 * 1024, 64, 1);
        cacheHierarchy = new CacheHierarchy(*l1Cache, *l2Cache);
        
        std::cout << "  Cache enabled: L1(32KB) --> L2(256KB)\n";
    }
    
    void setupVirtualMemory() {
        // Configure virtual memory with 4KB pages
        size_t pageSize = 4096;
        size_t numPhysicalFrames = std::max(size_t(1), memorySize / pageSize);
        size_t numVirtualPages = numPhysicalFrames * 4; // 4x overprovision
        
        vmManager = new VirtualMemoryManager(numVirtualPages, numPhysicalFrames, pageSize);
        
        std::cout << "  Virtual memory enabled:\n";
        std::cout << "    Page size: " << pageSize << " bytes\n";
        std::cout << "    Virtual pages: " << numVirtualPages << "\n";
        std::cout << "    Physical frames: " << numPhysicalFrames << "\n";
    }
    
    bool selectAllocator() {
        std::cout << "Select Memory Allocator:\n";
        std::cout << "  1. First Fit\n";
        std::cout << "  2. Best Fit\n";
        std::cout << "  3. Worst Fit\n";
        std::cout << "  4. Buddy System\n";
        std::cout << "\nEnter choice (1-4): ";
        
        int choice;
        if (!(std::cin >> choice)) {
            std::cerr << "Invalid input\n";
            return false;
        }
        std::cin.ignore();
        
        std::cout << "Enter memory size (bytes): ";
        if (!(std::cin >> memorySize)) {
            std::cerr << "Invalid size\n";
            return false;
        }
        std::cin.ignore();
        
        if (memorySize == 0) {
            std::cerr << "Error: Memory size must be greater than 0\n";
            return false;
        }
        
        try {
            switch (choice) {
                case 1:
                    allocator = new PhysicalMemory(memorySize, AllocationStrategy::FIRST_FIT);
                    std::cout << "\nInitialized " << memorySize << " bytes with First Fit allocator\n";
                    break;
                case 2:
                    allocator = new PhysicalMemory(memorySize, AllocationStrategy::BEST_FIT);
                    std::cout << "\nInitialized " << memorySize << " bytes with Best Fit allocator\n";
                    break;
                case 3:
                    allocator = new PhysicalMemory(memorySize, AllocationStrategy::WORST_FIT);
                    std::cout << "\nInitialized " << memorySize << " bytes with Worst Fit allocator\n";
                    break;
                case 4:
                    allocator = new BuddyAllocator(memorySize);
                    std::cout << "\nInitialized " << memorySize << " bytes with Buddy System allocator\n";
                    break;
                default:
                    std::cerr << "Invalid choice\n";
                    return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error initializing allocator: " << e.what() << "\n";
            return false;
        }
        
        return true;
    }
    
    /**
     * Simulates a memory access with full integration:
     * Flow: Virtual Address --> Page Table --> Physical Address --> Cache --> Memory
     * 
     * @param virtualAddr The virtual address to access
     * @param description Description of the operation for logging
     */
    void simulateMemoryAccess(uint64_t virtualAddr, const std::string& description) {
        std::cout << "  [" << description << "]\n";
        
        uint64_t physicalAddr = virtualAddr;
        
        // Step 1: Virtual Address Translation (if enabled)
        if (enableVirtualMemory) {
            std::cout << "    1. Virtual Address: 0x" << std::hex << std::setw(8) 
                      << std::setfill('0') << virtualAddr << std::dec << "\n";
            
            size_t faults_before = vmManager->page_faults();
            physicalAddr = vmManager->translate(virtualAddr);
            size_t faults_after = vmManager->page_faults();
            
            std::cout << "    2. Page Table Lookup --> Physical Address: 0x" 
                      << std::hex << std::setw(8) << std::setfill('0') 
                      << physicalAddr << std::dec << "\n";
            
            if (faults_after > faults_before) {
                std::cout << "       (Page fault occurred - page loaded into memory)\n";
            }
        } else {
            std::cout << "    1. Physical Address: 0x" << std::hex << std::setw(8) 
                      << std::setfill('0') << physicalAddr << std::dec << "\n";
        }
        
        // Step 2: Cache Access (if enabled)
        if (enableCache) {
            size_t l1_hits_before = cacheHierarchy->l1_hits();
            size_t l2_hits_before = cacheHierarchy->l2_hits();
            
            bool l1_hit = cacheHierarchy->access(physicalAddr);
            
            size_t l1_hits_after = cacheHierarchy->l1_hits();
            size_t l2_hits_after = cacheHierarchy->l2_hits();
            
            std::cout << "    " << (enableVirtualMemory ? "3" : "2") 
                      << ". Cache Access: ";
            
            if (l1_hit) {
                std::cout << "L1 HIT\n";
            } else if (l2_hits_after > l2_hits_before) {
                std::cout << "L1 MISS, L2 HIT\n";
            } else {
                std::cout << "L1 MISS, L2 MISS --> Memory Access\n";
            }
        } else {
            std::cout << "    " << (enableVirtualMemory ? "3" : "2") 
                      << ". Memory Access (no cache)\n";
        }
    }
    
    void processCommand(const std::string& line) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "malloc") {
            cmdMalloc(iss);
        } else if (cmd == "free") {
            cmdFree(iss);
        } else if (cmd == "access") {
            cmdAccess(iss);
        } else if (cmd == "dump") {
            cmdDump();
        } else if (cmd == "stats") {
            cmdStats();
        } else if (cmd == "cache_stats") {
            cmdCacheStats();
        } else if (cmd == "vm_stats") {
            cmdVMStats();
        } else if (cmd == "help") {
            cmdHelp();
        } else {
            std::cout << "Unknown command: " << cmd << "\n";
            std::cout << "Type 'help' for available commands\n";
        }
    }
    
    void cmdMalloc(std::istringstream& iss) {
        size_t size;
        
        if (!(iss >> size)) {
            std::cout << "Usage: malloc <size>\n";
            return;
        }
        
        if (size == 0) {
            std::cout << "Error: Size must be greater than 0\n";
            return;
        }
        
        int blockId = allocator->allocate(size);
        
        if (blockId != -1) {
            blockSizes[blockId] = size;
            std::cout << "Allocated block id=" << blockId << "\n";
            
            // Simulate memory access through the integration layers
            if (enableVirtualMemory || enableCache) {
                // Use block ID as a pseudo-address for demonstration
                // Use smaller spacing (256 bytes) to work with small memory sizes
                uint64_t addr = (blockId - 1) * 256;
                simulateMemoryAccess(addr, "Initial memory access after allocation");
            }
        } else {
            std::cout << "Error: Allocation failed - not enough memory\n";
        }
    }
    
    void cmdAccess(std::istringstream& iss) {
        uint64_t addr;
        
        if (!(iss >> std::hex >> addr)) {
            std::cout << "Usage: access <address_in_hex>\n";
            std::cout << "Example: access 0x1000\n";
            return;
        }
        
        if (!enableVirtualMemory && !enableCache) {
            std::cout << "Error: Virtual memory or cache must be enabled to use 'access' command\n";
            return;
        }
        
        simulateMemoryAccess(addr, "Manual memory access");
    }
    
    void cmdFree(std::istringstream& iss) {
        int blockId;
        
        if (!(iss >> blockId)) {
            std::cout << "Usage: free <block_id>\n";
            return;
        }
        
        auto it = blockSizes.find(blockId);
        if (it == blockSizes.end()) {
            std::cout << "Error: Block " << blockId << " not found\n";
            return;
        }
        
        allocator->free_block(blockId);
        blockSizes.erase(it);
        std::cout << "Block " << blockId << " freed and merged\n";
    }
    
    void cmdDump() {
        std::cout << "\n--- Memory Layout (" << allocator->allocator_name() << ") ---\n";
        allocator->dump();
        
        std::cout << "\n--- Allocated Blocks ---\n";
        if (blockSizes.empty()) {
            std::cout << "No allocated blocks\n";
        } else {
            for (const auto& entry : blockSizes) {
                std::cout << "Block id=" << entry.first 
                          << " size=" << entry.second << " bytes\n";
            }
        }
        std::cout << "\n";
    }
    
    void cmdStats() {
        size_t total = allocator->total_memory();
        size_t used = allocator->used_memory();
        size_t free = allocator->free_memory();
        
        double usagePercent = total > 0 ? (used * 100.0 / total) : 0.0;
        
        std::cout << "\n--- Memory Statistics ---\n";
        std::cout << "Allocator: " << allocator->allocator_name() << "\n";
        std::cout << "Total memory: " << total << "\n";
        std::cout << "Used memory: " << used << " (" 
                  << std::fixed << std::setprecision(2) << usagePercent << "%)\n";
        std::cout << "Free memory: " << free << "\n";
        std::cout << "Active allocations: " << blockSizes.size() << "\n";
        std::cout << "Largest free block: " << allocator->largest_free_block() << "\n";
        std::cout << "\n";
    }
    
    void cmdCacheStats() {
        if (!enableCache) {
            std::cout << "Cache not enabled. Use Y when prompted at startup.\n";
            return;
        }
        
        std::cout << "\n========================================\n";
        std::cout << "      Cache Hierarchy Statistics\n";
        std::cout << "========================================\n\n";
        
        // L1 Cache Statistics
        std::cout << "L1 Cache (32KB, 64-byte lines, direct-mapped):\n";
        std::cout << "  Hits:       " << std::setw(8) << cacheHierarchy->l1_hits() << "\n";
        std::cout << "  Misses:     " << std::setw(8) << cacheHierarchy->l1_misses() << "\n";
        
        size_t l1_total = cacheHierarchy->l1_hits() + cacheHierarchy->l1_misses();
        if (l1_total > 0) {
            double l1_hit_rate = (cacheHierarchy->l1_hits() * 100.0) / l1_total;
            double l1_miss_rate = (cacheHierarchy->l1_misses() * 100.0) / l1_total;
            std::cout << "  Total:      " << std::setw(8) << l1_total << "\n";
            std::cout << "  Hit Rate:   " << std::fixed << std::setprecision(2) 
                      << std::setw(7) << l1_hit_rate << "%\n";
            std::cout << "  Miss Rate:  " << std::fixed << std::setprecision(2) 
                      << std::setw(7) << l1_miss_rate << "%\n";
        } else {
            std::cout << "  Total:      " << std::setw(8) << 0 << "\n";
            std::cout << "  Hit Rate:        N/A\n";
        }
        
        // L2 Cache Statistics
        std::cout << "\nL2 Cache (256KB, 64-byte lines, direct-mapped):\n";
        std::cout << "  Hits:       " << std::setw(8) << cacheHierarchy->l2_hits() << "\n";
        std::cout << "  Misses:     " << std::setw(8) << cacheHierarchy->l2_misses() << "\n";
        
        size_t l2_total = cacheHierarchy->l2_hits() + cacheHierarchy->l2_misses();
        if (l2_total > 0) {
            double l2_hit_rate = (cacheHierarchy->l2_hits() * 100.0) / l2_total;
            double l2_miss_rate = (cacheHierarchy->l2_misses() * 100.0) / l2_total;
            std::cout << "  Total:      " << std::setw(8) << l2_total << "\n";
            std::cout << "  Hit Rate:   " << std::fixed << std::setprecision(2) 
                      << std::setw(7) << l2_hit_rate << "%\n";
            std::cout << "  Miss Rate:  " << std::fixed << std::setprecision(2) 
                      << std::setw(7) << l2_miss_rate << "%\n";
        } else {
            std::cout << "  Total:      " << std::setw(8) << 0 << "\n";
            std::cout << "  Hit Rate:        N/A\n";
        }
        
        // Overall Statistics and Miss Penalty Analysis
        std::cout << "\n--- Miss Penalty Propagation ---\n";
        size_t l1_misses = cacheHierarchy->l1_misses();
        size_t l2_hits = cacheHierarchy->l2_hits();
        size_t l2_misses = cacheHierarchy->l2_misses();
        
        std::cout << "L1 Misses:              " << std::setw(8) << l1_misses << "\n";
        std::cout << "  Serviced by L2:    " << std::setw(8) << l2_hits 
                  << " (L2 hits)\n";
        std::cout << "  Propagated to RAM: " << std::setw(8) << l2_misses 
                  << " (L2 misses)\n";
        
        if (l1_misses > 0) {
            double l2_hit_on_l1_miss = (l2_hits * 100.0) / l1_misses;
            double mem_access_rate = (l2_misses * 100.0) / l1_misses;
            std::cout << "\nL2 Hit Rate (on L1 miss): " << std::fixed << std::setprecision(2)
                      << l2_hit_on_l1_miss << "%\n";
            std::cout << "Memory Access Rate:       " << std::fixed << std::setprecision(2)
                      << mem_access_rate << "%\n";
        }
        
        // Simulated Access Times (in cycles)
        std::cout << "\n--- Simulated Access Latencies ---\n";
        std::cout << "L1 Hit:         1 cycle\n";
        std::cout << "L2 Hit:        10 cycles (L1 miss + L2 access)\n";
        std::cout << "L2 Miss:      100 cycles (L1 miss + L2 miss + RAM access)\n";
        
        if (l1_total > 0) {
            // Calculate average memory access time
            double avg_cycles = 
                (cacheHierarchy->l1_hits() * 1.0) +      // L1 hits: 1 cycle
                (l2_hits * 10.0) +                        // L2 hits: 10 cycles
                (l2_misses * 100.0);                      // RAM access: 100 cycles
            
            double amat = avg_cycles / l1_total;
            std::cout << "\nAverage Memory Access Time (AMAT): " 
                      << std::fixed << std::setprecision(2) << amat << " cycles\n";
        }
        
        std::cout << "\n========================================\n\n";
    }
    
    void cmdVMStats() {
        if (!enableVirtualMemory) {
            std::cout << "Virtual memory not enabled. Use Y when prompted at startup.\n";
            return;
        }
        
        std::cout << "\n--- Virtual Memory Statistics ---\n";
        std::cout << "Page faults: " << vmManager->page_faults() << "\n";
        std::cout << "Total accesses: " << vmManager->page_faults() + vmManager->page_faults() << "\n";
        std::cout << "\n";
    }
    
    void cmdHelp() {
        std::cout << "\n=== Available Commands ===\n\n";
        std::cout << "Allocation Operations:\n";
        std::cout << "  malloc <size>         - Allocate memory block\n";
        std::cout << "  free <block_id>       - Free allocated block\n\n";
        
        std::cout << "Visualization:\n";
        std::cout << "  dump                  - Show memory layout\n";
        std::cout << "  stats                 - Show statistics\n\n";
        
        if (enableCache || enableVirtualMemory) {
            std::cout << "Memory Access & Integration:\n";
            if (enableVirtualMemory) {
                std::cout << "  access <vaddr>        - Access virtual address (translation & cache)\n";
                std::cout << "  vm_stats              - Show virtual memory statistics\n";
            }
            if (enableCache) {
                std::cout << "  cache_stats           - Show cache hit/miss statistics\n";
            }
            std::cout << "\n";
        }
        
        std::cout << "General:\n";
        std::cout << "  help                  - Show this help\n";
        std::cout << "  exit/quit             - Exit simulator\n\n";
        
        std::cout << "Current allocator: " << allocator->allocator_name() << "\n\n";
    }
};

int main() {
    MemorySimulatorCLI cli;
    cli.run();
    return 0;
}

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
