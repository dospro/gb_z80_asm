#include "allocator.h"

#include <stdlib.h>


/**
 * Allocates from the C heap, ignoring the allocator context.
 *
 * Adapts malloc to the Allocator signature. Kept private because callers reach
 * it only through allocator_default().
 *
 * @param context Unused; the C heap keeps no per-instance state.
 * @param size Number of bytes to allocate.
 * @return Allocated storage, or nullptr when the allocation fails.
 */
static void* malloc_alloc(void* const context, const size_t size)
{
    (void)context;
    return malloc(size);
}

/**
 * Releases storage obtained from malloc_alloc, ignoring the allocator context.
 *
 * Adapts free to the Allocator signature. Kept private because callers reach it
 * only through allocator_default().
 *
 * @param context Unused; the C heap keeps no per-instance state.
 * @param pointer Storage returned by malloc_alloc; nullptr is ignored.
 */
static void malloc_free(void* const context, void* const pointer)
{
    (void)context;
    free(pointer);
}

Allocator allocator_default()
{
    return (Allocator){.alloc = malloc_alloc, .free = malloc_free, .context = nullptr};
}