#include "symbol.h"

#include <stdlib.h>

#include "string.h"


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

SymbolTable SymbolTable_new(const size_t capacity)
{
    return SymbolTable_new_with_allocator(capacity, allocator_default());
}

SymbolTable SymbolTable_new_with_allocator(const size_t capacity, const Allocator allocator)
{
    if (capacity > SIZE_MAX / sizeof(Symbol))
    {
        return (SymbolTable){.allocator = allocator};
    }
    Symbol* const symbols = allocator.alloc(allocator.context, capacity * sizeof(Symbol));
    return (SymbolTable){
        .symbols = symbols,
        .capacity = symbols == nullptr ? 0 : capacity,
        .size = 0,
        .allocator = allocator,
    };
}


void SymbolTable_free(SymbolTable* const table)
{
    if (table == nullptr) return;
    if (table->symbols != nullptr)
    {
        table->allocator.free(table->allocator.context, table->symbols);
        table->symbols = nullptr;
    }
    table->size = 0;
    table->capacity = 0;
}

bool SymbolTable_add_symbol(SymbolTable* const table, const Symbol symbol)
{
    if (table == nullptr)
    {
        return false;
    }
    if (table->size >= table->capacity)
    {
        const size_t new_capacity = table->capacity == 0 ? 4 : table->capacity * 2;
        if (new_capacity > SIZE_MAX / sizeof(Symbol))
        {
            return false;
        }
        Symbol* new_array = table->allocator.alloc(table->allocator.context, new_capacity * sizeof(Symbol));
        if (new_array == nullptr) return false;
        for (size_t i = 0; i < table->size; i++)
        {
            new_array[i] = table->symbols[i];
        }
        table->allocator.free(table->allocator.context, table->symbols);
        table->symbols = new_array;
        table->capacity = new_capacity;
    }

    table->symbols[table->size] = symbol;
    table->size++;

    return true;
}
