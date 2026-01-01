#ifndef PHYSICAL_MEMORY_H
#define PHYSICAL_MEMORY_H

#include "IAllocator.h"
#include <cstddef>
#include <list>
#include <string>

struct MemoryBlock {
    std::size_t start;
    std::size_t size;
    bool free;
    int id;
};

enum class AllocationStrategy {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
};

class PhysicalMemory : public IAllocator {
public:
    explicit PhysicalMemory(std::size_t total_size, AllocationStrategy strategy = AllocationStrategy::FIRST_FIT);

    // Allocation strategies
    int allocate_first_fit(std::size_t size);
    int allocate_best_fit(std::size_t size);
    int allocate_worst_fit(std::size_t size);

    // Strategy management
    void set_strategy(AllocationStrategy strategy);
    AllocationStrategy get_strategy() const;

    // IAllocator interface implementation
    int allocate(std::size_t size) override;
    void free_block(int id) override;
    std::size_t total_memory() const override;
    std::size_t used_memory() const override;
    std::size_t free_memory() const override;
    std::size_t largest_free_block() const override;
    void dump() const override;
    const char* allocator_name() const override;

    // Additional functionality
    double external_fragmentation() const;

private:
    std::size_t total_size_;
    std::list<MemoryBlock> blocks_;
    int next_id_;
    AllocationStrategy strategy_;
    int allocate_from_block(std::list<MemoryBlock>::iterator it, std::size_t size);
};

#endif
