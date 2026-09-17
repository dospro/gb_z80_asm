#include "symbol.h"

#include "allocator.h"
#include "vector.h"

SymbolTable SymbolTable_new(const size_t capacity)
{
    return SymbolTable_new_with_allocator(capacity, allocator_default());
}

SymbolTable SymbolTable_new_with_allocator(const size_t capacity, const Allocator allocator)
{
    return (SymbolTable){.symbols = Vector_new_with_allocator(sizeof(Symbol), capacity, allocator)};
}

void SymbolTable_free(SymbolTable* const table)
{
    if (table == nullptr) return;
    Vector_free(&table->symbols);
}

bool SymbolTable_add_symbol(SymbolTable* const table, const Symbol symbol)
{
    return table != nullptr && Vector_push(&table->symbols, &symbol);
}

Symbol* SymbolTable_at(const SymbolTable* const table, const size_t index)
{
    return table == nullptr ? nullptr : Vector_at(&table->symbols, index);
}
