#ifndef PHYSICAL_MEMORY_H
#define PHYSICAL_MEMORY_H

#include <cstddef>
#include <list>

struct MemoryBlock {
    std::size_t start;
    std::size_t size;
    bool free;
    int id;
};

class PhysicalMemory {
public:
    explicit PhysicalMemory(std::size_t total_size);

    int allocate_first_fit(std::size_t size);
    int allocate_best_fit(std::size_t size);
    int allocate_worst_fit(std::size_t size);

    std::size_t total_memory() const;
    std::size_t used_memory() const;
    std::size_t free_memory() const;
    std::size_t largest_free_block() const;
    double external_fragmentation() const;


    void dump() const;
    void free_block(int id);

private:
    std::size_t total_size_;
    std::list<MemoryBlock> blocks_;
    int next_id_;
    int allocate_from_block(std::list<MemoryBlock>::iterator it, std::size_t size);

    
};

#endif
