#ifndef IALLOCATOR_H
#define IALLOCATOR_H

#include <cstddef>

/**
 * Abstract interface for memory allocators
 * Provides a common interface for different allocation strategies
 */
class IAllocator {
public:
    virtual ~IAllocator() = default;
    
    // Core allocation functions
    virtual int allocate(std::size_t size) = 0;
    virtual void free_block(int id) = 0;
    
    // Memory information
    virtual std::size_t total_memory() const = 0;
    virtual std::size_t used_memory() const = 0;
    virtual std::size_t free_memory() const = 0;
    virtual std::size_t largest_free_block() const = 0;
    
    // Visualization
    virtual void dump() const = 0;
    
    // Allocator identification
    virtual const char* allocator_name() const = 0;
};

#endif // IALLOCATOR_H
