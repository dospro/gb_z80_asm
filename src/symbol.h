#ifndef GB_Z80_ASM_SYMBOL_H
#define GB_Z80_ASM_SYMBOL_H
#include <stdlib.h>

#include "string.h"

typedef struct Allocator Allocator;

/**
 * A pair of allocation functions together with the state they operate on.
 *
 * A SymbolTable stores an Allocator by value and calls through it for every
 * allocation and release, so callers can substitute an arena, a
 * failure-injecting test double, or anything else without the table knowing.
 * Both functions receive @c context unchanged; the table never inspects it.
 *
 * The allocator, and whatever @c context points at, must stay valid for as long
 * as any object built from it is alive. @c free must accept storage returned by
 * the matching @c alloc.
 */
struct Allocator
{
    /** Returns @p size bytes, or nullptr on failure. Receives @c context. */
    void* (*alloc)(void* context, size_t size);
    /** Releases storage from @c alloc. Receives @c context. */
    void (*free)(void* context, void* pointer);
    /** Opaque state handed back to @c alloc and @c free; may be nullptr. */
    void* context;
};

/**
 * Returns the allocator backed by the C heap.
 *
 * Its @c context is nullptr, because malloc and free keep no per-instance
 * state. This is the allocator SymbolTable_new uses.
 *
 * @return An allocator that is always valid and requires no teardown.
 */
Allocator allocator_default();

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
 * A growable vector of symbols that carries its own allocator.
 *
 * The table owns the @c symbols array and releases it in SymbolTable_free. It
 * does not own the text behind each symbol's name; see Symbol.
 *
 * Tables are passed and returned by value. The allocator travels with the
 * table, so a copy can always grow or release its own storage, but two copies
 * of the same table must not both free it.
 */
struct SymbolTable
{
    /** Storage for @c capacity symbols, or nullptr when none is held. */
    Symbol* symbols;
    /** Symbols currently stored; never greater than @c capacity. */
    size_t size;
    /** Number of symbols @c symbols has room for. */
    size_t capacity;
    /** Allocator used for every allocation and release of @c symbols. */
    Allocator allocator;
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
 * The allocator is copied into the returned table and is used for every later
 * allocation and release, including the one performed by SymbolTable_free.
 *
 * Failure is reported through the returned table rather than a separate status:
 * @c symbols is nullptr and @c capacity is 0. A @p capacity whose byte size
 * would overflow size_t is rejected the same way, without calling the allocator
 * at all. In both cases the allocator is still stored, so the returned table can
 * be freed safely and can still acquire storage later.
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
 * The array is freed through the table's own allocator. The text behind each
 * symbol's name is deliberately left alone, because the table never owned it.
 *
 * Calling this twice is safe, as is calling it on a table whose creation
 * failed: @c symbols is cleared and the counts zeroed, so a second call has
 * nothing to do. The allocator is left in place, so the table can be reused.
 *
 * @param table Table to empty; nullptr is ignored.
 */
void SymbolTable_free(SymbolTable* table);
bool SymbolTable_add_symbol(SymbolTable* table, Symbol symbol);

#endif //GB_Z80_ASM_SYMBOL_H
