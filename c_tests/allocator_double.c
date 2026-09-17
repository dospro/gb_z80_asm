#include "allocator_double.h"

#include <stdlib.h>

void* controlled_alloc(void* const context, const size_t size)
{
    AllocationControl* const control = context;
    control->alloc_calls++;
    return control->fail_allocations ? nullptr : malloc(size);
}

void controlled_free(void* const context, void* const pointer)
{
    AllocationControl* const control = context;
    control->free_calls++;
    free(pointer);
}
