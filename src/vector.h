#ifndef GB_Z80_ASM_VECTOR_H
#define GB_Z80_ASM_VECTOR_H
#include <stddef.h>

#include "allocator.h"

typedef struct Vector Vector;

/**
 * A growable array of fixed-size elements that carries its own allocator.
 *
 * The vector is type-erased: it knows only how many bytes one element takes,
 * and copies elements in and out as raw bytes. Typed containers such as
 * SymbolTable and FixupTable wrap a Vector and pin the element type at their
 * own API boundary, so callers never see @c void* themselves.
 *
 * The vector owns the @c data block and releases it in Vector_free. It knows
 * nothing about what the elements point at; if an element holds a pointer or
 * a String view, the vector neither copies nor frees that referent.
 *
 * Vectors are passed and returned by value. The allocator travels with the
 * vector, so a copy can always grow or release its own storage, but two
 * copies of the same vector must not both free it.
 *
 * @c data is placed by whatever the allocator returns, so the allocator must
 * hand back storage aligned for the element type; the C heap always does.
 */
struct Vector
{
    /** Storage for @c capacity elements of @c element_size bytes each, or nullptr when none is held. */
    void* data;
    /** Elements currently stored; never greater than @c capacity. */
    size_t size;
    /** Number of elements @c data has room for. */
    size_t capacity;
    /** Bytes per element, fixed at creation; never zero on a usable vector. */
    size_t element_size;
    /** Allocator used for every allocation and release of @c data. */
    Allocator allocator;
};

/**
 * Creates an empty vector with room for @p capacity elements, using the C heap.
 *
 * Equivalent to Vector_new_with_allocator called with allocator_default().
 *
 * @param element_size Bytes per element; must not be zero.
 * @param capacity Number of elements to reserve room for; 0 is valid.
 * @return A vector with size 0. See Vector_new_with_allocator for how failure
 *         is reported.
 */
Vector Vector_new(size_t element_size, size_t capacity);

/**
 * Creates an empty vector with room for @p capacity elements, using
 * @p allocator.
 *
 * The allocator is copied into the returned vector and is used for every
 * later allocation and release, including the one performed by Vector_free.
 *
 * Failure is reported through the returned vector rather than a separate
 * status: @c data is nullptr and @c capacity is 0. A @p capacity whose byte
 * size would overflow size_t is rejected the same way, without calling the
 * allocator at all. In both cases the allocator and element size are still
 * stored, so the returned vector can be freed safely and can still acquire
 * storage later.
 *
 * A @p capacity of 0 also skips the allocator and yields @c data nullptr, so
 * an empty vector always has null storage rather than whatever the allocator
 * returns for a zero-byte request. The first push allocates.
 *
 * A zero @p element_size is rejected outright: the returned vector has
 * @c element_size 0 as well, and every later push on it fails.
 *
 * @param element_size Bytes per element; must not be zero.
 * @param capacity Number of elements to reserve room for; 0 is valid.
 * @param allocator Allocation functions and context to store in the vector;
 *        must outlive the vector.
 * @return A vector with size 0, left empty if the reservation could not be
 *         made.
 */
Vector Vector_new_with_allocator(size_t element_size, size_t capacity, Allocator allocator);

/**
 * Releases the vector's storage and resets it to an empty vector.
 *
 * The block is freed through the vector's own allocator. Whatever the
 * elements themselves referred to is deliberately left alone, because the
 * vector never owned it.
 *
 * Calling this twice is safe, as is calling it on a vector whose creation
 * failed: @c data is cleared and the counts zeroed, so a second call has
 * nothing to do. The allocator and element size are left in place, so the
 * vector can be reused.
 *
 * @param vector Vector to empty; nullptr is ignored.
 */
void Vector_free(Vector* vector);

/**
 * Appends a copy of the bytes at @p element to the end of the vector, growing
 * it if needed.
 *
 * Exactly @c element_size bytes are read from @p element. When the vector is
 * full its storage is doubled (starting from 4 when empty) through the
 * vector's allocator, and the existing elements are copied across. The
 * address of @c data may therefore change; callers must not hold pointers
 * obtained from Vector_at across this call.
 *
 * On failure the vector is left exactly as it was: nothing is appended, size
 * and capacity are unchanged, and the existing storage is still valid. Growth
 * fails when the allocator returns nullptr or when the doubled byte size
 * would overflow size_t.
 *
 * @param vector Vector to append to; nullptr is rejected.
 * @param element Bytes to copy in; nullptr is rejected.
 * @return true if the element was stored, false otherwise.
 */
bool Vector_push(Vector* vector, const void* element);

/**
 * Returns the address of the element at @p index.
 *
 * The bound is @c size, not @c capacity: storage past the last pushed element
 * exists but has never been written, and is never handed out.
 *
 * The returned pointer is valid until the next call that may grow the vector
 * or until Vector_free, whichever comes first. It points into the vector's
 * own storage, so writing through it modifies the stored element.
 *
 * @param vector Vector to read from; nullptr is rejected.
 * @param index Zero-based position of the element.
 * @return Address of the element, or nullptr when @p index is at or past
 *         @c size.
 */
void* Vector_at(const Vector* vector, size_t index);

#endif //GB_Z80_ASM_VECTOR_H
