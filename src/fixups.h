#ifndef GB_Z80_ASM_FIXUP_H
#define GB_Z80_ASM_FIXUP_H
#include "allocator.h"
#include "string.h"
#include "vector.h"


typedef enum FixupKind FixupKind;

/**
 * How the placeholder bytes at a fixup's address must be patched once the
 * label it names is known.
 */
enum FixupKind
{
    /** Two little-endian bytes holding the label's absolute address (call, jp nn). */
    FIXUP_ABSOLUTE_16,
    /** One signed byte holding the label's offset from the next instruction (jr n). */
    FIXUP_RELATIVE_8,
};

typedef struct Fixup Fixup;

/**
 * One place in the ROM image that refers to a label by name and still holds
 * placeholder bytes.
 *
 * Fixups are recorded during the first pass, when an instruction names a label
 * that may not be defined yet. The second pass looks each label up in the
 * SymbolTable and overwrites the placeholder according to @c kind.
 *
 * A Fixup does not own its label. @c label is a view into the source buffer the
 * assembler read, so a fixup is only usable while that buffer is alive and has
 * not been reallocated; growing the source buffer invalidates the labels of
 * every fixup already recorded.
 */
struct Fixup
{
    /** Label text the instruction referred to; a non-owning view into the source buffer. */
    String label;
    /** ROM bank number the placeholder bytes live in. */
    unsigned int bank;
    /** Byte offset in the ROM image of the first placeholder byte. */
    unsigned int address;
    /** 1-based source line the reference appeared on, used for diagnostics. */
    unsigned int line;
    /** Encoding to apply when the placeholder is patched. */
    FixupKind kind;
};

typedef struct FixupTable FixupTable;

/**
 * A growable table of fixups, backed by a Vector of Fixup.
 *
 * The table is a typed wrapper: it pins the element type to Fixup at every
 * function below, so callers never deal with the underlying @c void* storage.
 * Ownership, growth, and failure behavior are the Vector's; see vector.h. In
 * short, the table owns @c fixups and releases it in FixupTable_free, but does
 * not own the text behind each fixup's label; see Fixup.
 *
 * Tables are passed and returned by value, with the same caveat as Vector: two
 * copies of the same table must not both free it.
 */
struct FixupTable
{
    /** Element storage; every element is a Fixup. Read @c size from here. */
    Vector fixups;
};


/**
 * Creates an empty table with room for @p capacity fixups, using the C heap.
 *
 * Equivalent to FixupTable_new_with_allocator called with allocator_default().
 *
 * @param capacity Number of fixups to reserve room for; 0 is valid.
 * @return A table with size 0. See FixupTable_new_with_allocator for how
 *         failure is reported.
 */
FixupTable FixupTable_new(size_t capacity);

/**
 * Creates an empty table with room for @p capacity fixups, using @p allocator.
 *
 * Forwards to Vector_new_with_allocator with sizeof(Fixup); failure is
 * reported the same way, through the returned table's @c fixups vector.
 *
 * @param capacity Number of fixups to reserve room for; 0 is valid.
 * @param allocator Allocation functions and context to store in the table; must
 *        outlive the table.
 * @return A table with size 0, left empty if the reservation could not be made.
 */
FixupTable FixupTable_new_with_allocator(size_t capacity, Allocator allocator);

/**
 * Releases the table's fixup storage and resets it to an empty table.
 *
 * Forwards to Vector_free. The text behind each fixup's label is deliberately
 * left alone, because the table never owned it.
 *
 * @param table Table to empty; nullptr is ignored.
 */
void FixupTable_free(FixupTable* table);

/**
 * Appends a copy of @p fixup to the end of the table, growing it if needed.
 *
 * Forwards to Vector_push. Pointers previously obtained from FixupTable_at
 * may be invalidated by this call.
 *
 * @param table Table to append to; nullptr is rejected.
 * @param fixup Fixup to copy in. Its label is stored as the same view, not
 *        duplicated.
 * @return true if the fixup was stored, false otherwise.
 */
bool FixupTable_add_fixup(FixupTable* table, Fixup fixup);

/**
 * Returns the fixup at @p index.
 *
 * Forwards to Vector_at with the result typed as Fixup. The pointer points
 * into the table's own storage and is valid until the next
 * FixupTable_add_fixup or FixupTable_free.
 *
 * @param table Table to read from; nullptr is rejected.
 * @param index Zero-based position of the fixup.
 * @return The fixup, or nullptr when @p index is at or past the table's size.
 */
Fixup* FixupTable_at(const FixupTable* table, size_t index);

#endif //GB_Z80_ASM_FIXUP_H