#include "fixups.h"

#include "allocator.h"
#include "vector.h"

FixupTable FixupTable_new(const size_t capacity)
{
    return FixupTable_new_with_allocator(capacity, allocator_default());
}

FixupTable FixupTable_new_with_allocator(const size_t capacity, const Allocator allocator)
{
    return (FixupTable){.fixups = Vector_new_with_allocator(sizeof(Fixup), capacity, allocator)};
}

void FixupTable_free(FixupTable* const table)
{
    if (table == nullptr) return;
    Vector_free(&table->fixups);
}

bool FixupTable_add_fixup(FixupTable* const table, const Fixup fixup)
{
    return table != nullptr && Vector_push(&table->fixups, &fixup);
}

Fixup* FixupTable_at(const FixupTable* const table, const size_t index)
{
    return table == nullptr ? nullptr : Vector_at(&table->fixups, index);
}
