#ifndef GB_Z80_ASM_ALLOCATOR_DOUBLE_H
#define GB_Z80_ASM_ALLOCATOR_DOUBLE_H

#include <stddef.h>
#include <stdbool.h>

#include "../src/allocator.h"

/**
 * Records what the injected test allocator was asked to do.
 *
 * controlled_alloc and controlled_free reach this block through the
 * Allocator's context pointer rather than a global, so each test owns its own
 * counters and tests cannot interfere with one another. Counting allocations
 * and frees separately lets a test prove that the object under test used the
 * allocator it was given, and that it returned the storage through the
 * matching free.
 */
typedef struct AllocationControl AllocationControl;

struct AllocationControl
{
    size_t alloc_calls;
    size_t free_calls;
    bool fail_allocations;
};

/**
 * Allocator alloc function for use with an AllocationControl context.
 *
 * Counts the call, then forwards to malloc, or returns nullptr without
 * allocating when @c fail_allocations is set.
 *
 * @param context Must point at the AllocationControl to record into.
 * @param size Bytes requested.
 * @return Storage from malloc, or nullptr when failure is forced.
 */
void* controlled_alloc(void* context, size_t size);

/**
 * Allocator free function for use with an AllocationControl context.
 *
 * Counts the call, then forwards to free.
 *
 * @param context Must point at the AllocationControl to record into.
 * @param pointer Storage previously returned by controlled_alloc.
 */
void controlled_free(void* context, void* pointer);

#endif //GB_Z80_ASM_ALLOCATOR_DOUBLE_H
