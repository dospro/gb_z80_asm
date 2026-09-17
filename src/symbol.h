#ifndef GB_Z80_ASM_SYMBOL_H
#define GB_Z80_ASM_SYMBOL_H
#include "allocator.h"
#include "string.h"
#include "vector.h"

typedef struct Symbol Symbol;

/**
 * One named location in the assembled program.
 *
 * A Symbol does not own its name. @c name is a view into the source buffer the
 * assembler read, so a symbol is only usable while that buffer is alive and has
 * not been reallocated; growing the source buffer invalidates the names of
 * every symbol already recorded.
 */
struct Symbol
{
    /** Label text; a non-owning view into the source buffer. */
    String name;
    /** Rom bank number where the symbol is encoded */
    unsigned int bank;
    /** Byte offset in the ROM image that the label resolves to. */
    unsigned int address;
    /** 1-based source line the label was defined on, used for diagnostics. */
    unsigned int definition_line;
};

typedef struct SymbolTable SymbolTable;

/**
 * A growable table of symbols, backed by a Vector of Symbol.
 *
 * The table is a typed wrapper: it pins the element type to Symbol at every
 * function below, so callers never deal with the underlying @c void* storage.
 * Ownership, growth, and failure behavior are the Vector's; see vector.h. In
 * short, the table owns @c symbols and releases it in SymbolTable_free, but
 * does not own the text behind each symbol's name; see Symbol.
 *
 * Tables are passed and returned by value, with the same caveat as Vector: two
 * copies of the same table must not both free it.
 */
struct SymbolTable
{
    /** Element storage; every element is a Symbol. Read @c size from here. */
    Vector symbols;
};

/**
 * Creates an empty table with room for @p capacity symbols, using the C heap.
 *
 * Equivalent to SymbolTable_new_with_allocator called with allocator_default().
 *
 * @param capacity Number of symbols to reserve room for; 0 is valid.
 * @return A table with size 0. See SymbolTable_new_with_allocator for how
 *         failure is reported.
 */
SymbolTable SymbolTable_new(size_t capacity);

/**
 * Creates an empty table with room for @p capacity symbols, using @p allocator.
 *
 * Forwards to Vector_new_with_allocator with sizeof(Symbol); failure is
 * reported the same way, through the returned table's @c symbols vector.
 *
 * @param capacity Number of symbols to reserve room for; 0 is valid.
 * @param allocator Allocation functions and context to store in the table; must
 *        outlive the table.
 * @return A table with size 0, left empty if the reservation could not be made.
 */
SymbolTable SymbolTable_new_with_allocator(size_t capacity, Allocator allocator);

/**
 * Releases the table's symbol storage and resets it to an empty table.
 *
 * Forwards to Vector_free. The text behind each symbol's name is deliberately
 * left alone, because the table never owned it.
 *
 * @param table Table to empty; nullptr is ignored.
 */
void SymbolTable_free(SymbolTable* table);

/**
 * Appends a copy of @p symbol to the end of the table, growing it if needed.
 *
 * Forwards to Vector_push. Pointers previously obtained from SymbolTable_at
 * may be invalidated by this call.
 *
 * No duplicate check is performed; a label defined twice yields two entries.
 *
 * @param table Table to append to; nullptr is rejected.
 * @param symbol Symbol to copy in. Its name is stored as the same view, not
 *        duplicated.
 * @return true if the symbol was stored, false otherwise.
 */
bool SymbolTable_add_symbol(SymbolTable* table, Symbol symbol);

/**
 * Returns the symbol at @p index.
 *
 * Forwards to Vector_at with the result typed as Symbol. The pointer points
 * into the table's own storage and is valid until the next
 * SymbolTable_add_symbol or SymbolTable_free.
 *
 * @param table Table to read from; nullptr is rejected.
 * @param index Zero-based position of the symbol.
 * @return The symbol, or nullptr when @p index is at or past the table's size.
 */
Symbol* SymbolTable_at(const SymbolTable* table, size_t index);

#endif //GB_Z80_ASM_SYMBOL_H
