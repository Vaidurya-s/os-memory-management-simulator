#include "allocator/PhysicalMemory.h"
#include <iostream>

PhysicalMemory::PhysicalMemory(std::size_t total_size)
    : total_size_(total_size), next_id_(1)
{
    MemoryBlock initial;
    initial.start = 0;
    initial.size = total_size_;
    initial.free = true;
    initial.id = -1;

    blocks_.push_back(initial);
}


void PhysicalMemory::dump() const
{
    std::cout << "Physical Memory Dump" << std::endl;
    for (const auto& block : blocks_) {
        std::cout << "["
                  << block.start << " - "
                  << (block.start + block.size - 1) << "] ";

        if (block.free) {
            std::cout << "FREE";
        } else {
            std::cout << "USED (id=" << block.id << ")";
        }

        std::cout << std::endl;
    }
}



void PhysicalMemory::free_block(int id)
{
    for (auto it = blocks_.begin(); it != blocks_.end(); ++it) {
        if (!it->free && it->id == id) {

            it->free = true;
            it->id = -1;

            // Merge with previous block if free
            if (it != blocks_.begin()) {
                auto prev = std::prev(it);
                if (prev->free) {
                    prev->size += it->size;
                    it = blocks_.erase(it);
                    it = prev;
                }
            }

            // Merge with next block if free
            auto next = std::next(it);
            if (next != blocks_.end() && next->free) {
                it->size += next->size;
                blocks_.erase(next);
            }

            return;
        }
    }
}


int PhysicalMemory::allocate_from_block(std::list<MemoryBlock>::iterator it, std::size_t size)
{
    int allocated_id = next_id_++;

    if (it->size == size) {
        it->free = false;
        it->id = allocated_id;
    } else {
        MemoryBlock allocated;
        allocated.start = it->start;
        allocated.size = size;
        allocated.free = false;
        allocated.id = allocated_id;

        it->start += size;
        it->size -= size;

        blocks_.insert(it, allocated);
    }

    return allocated_id;
}


int PhysicalMemory::allocate_first_fit(std::size_t size)
{
    for (auto it = blocks_.begin(); it != blocks_.end(); ++it) {
        if (it->free && it->size >= size) {
            return allocate_from_block(it, size);
        }
    }
    return -1;
}


int PhysicalMemory::allocate_best_fit(std::size_t size)
{
    auto best = blocks_.end();

    for (auto it = blocks_.begin(); it != blocks_.end(); ++it) {
        if (it->free && it->size >= size) {
            if (best == blocks_.end() || it->size < best->size) {
                best = it;
            }
        }
    }

    if (best != blocks_.end()) {
        return allocate_from_block(best, size);
    }

    return -1;
}


int PhysicalMemory::allocate_worst_fit(std::size_t size)
{
    auto worst = blocks_.end();

    for (auto it = blocks_.begin(); it != blocks_.end(); ++it) {
        if (it->free && it->size >= size) {
            if (worst == blocks_.end() || it->size > worst->size) {
                worst = it;
            }
        }
    }

    if (worst != blocks_.end()) {
        return allocate_from_block(worst, size);
    }

    return -1;
}


std::size_t PhysicalMemory::total_memory() const
{
    return total_size_;
}

std::size_t PhysicalMemory::used_memory() const
{
    std::size_t used = 0;
    for (const auto& block : blocks_) {
        if (!block.free) {
            used += block.size;
        }
    }
    return used;
}

std::size_t PhysicalMemory::free_memory() const
{
    return total_size_ - used_memory();
}

std::size_t PhysicalMemory::largest_free_block() const
{
    std::size_t largest = 0;
    for (const auto& block : blocks_) {
        if (block.free && block.size > largest) {
            largest = block.size;
        }
    }
    return largest;
}

double PhysicalMemory::external_fragmentation() const
{
    std::size_t free_mem = free_memory();
    if (free_mem == 0) {
        return 0.0;
    }

    std::size_t largest = largest_free_block();
    return 1.0 - static_cast<double>(largest) / static_cast<double>(free_mem);
}
