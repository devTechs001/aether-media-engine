// ═══════════════════════════════════════════════════════════════════════════════
// FILE: include/aether/utils/memory.hpp
// DESCRIPTION: Memory utilities
// ═══════════════════════════════════════════════════════════════════════════════

#ifndef AETHER_UTILS_MEMORY_HPP
#define AETHER_UTILS_MEMORY_HPP

#include "aether/export.hpp"
#include "aether/core/types.hpp"

#include <memory>
#include <cstddef>

namespace aether {

/**
 * @class MemoryPool
 * @brief Memory pool for efficient allocation
 */
class AETHER_API MemoryPool {
public:
    explicit MemoryPool(usize block_size, usize num_blocks);
    ~MemoryPool();

    /**
     * @brief Allocate memory
     */
    void* Allocate();

    /**
     * @brief Deallocate memory
     */
    void Deallocate(void* ptr);

    /**
     * @brief Get available blocks
     */
    [[nodiscard]] usize AvailableBlocks() const;

    /**
     * @brief Get total blocks
     */
    [[nodiscard]] usize TotalBlocks() const;

    /**
     * @brief Reset pool
     */
    void Reset();
};

/**
 * @brief Get current memory usage
 */
AETHER_API usize GetMemoryUsage();

/**
 * @brief Get video memory usage
 */
AETHER_API usize GetVideoMemoryUsage();

/**
 * @brief Align size to cache line
 */
constexpr usize AlignToCacheLine(usize size) {
    return (size + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
}

/**
 * @brief Aligned allocator
 */
template<typename T, usize Alignment = alignof(T)>
class AlignedAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;

    template<typename U>
    struct rebind {
        using other = AlignedAllocator<U, Alignment>;
    };

    AlignedAllocator() = default;

    template<typename U>
    AlignedAllocator(const AlignedAllocator<U, Alignment>&) {}

    pointer allocate(usize n) {
        return static_cast<pointer>(::operator new(n * sizeof(T), 
            std::align_val_t(Alignment)));
    }

    void deallocate(pointer p, usize) {
        ::operator delete(p, std::align_val_t(Alignment));
    }
};

template<typename T, usize Alignment>
bool operator==(const AlignedAllocator<T, Alignment>&, 
                const AlignedAllocator<T, Alignment>&) {
    return true;
}

} // namespace aether

#endif // AETHER_UTILS_MEMORY_HPP
