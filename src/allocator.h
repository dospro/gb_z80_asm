#ifndef GB_Z80_ASM_ALLOCATOR_H
#define GB_Z80_ASM_ALLOCATOR_H
#include <stddef.h>

typedef struct Allocator Allocator;

/**
 * A pair of allocation functions together with the state they operate on.
 *
 * Owning containers such as Vector store an Allocator by value and call
 * through it for every allocation and release, so callers can substitute an
 * arena, a failure-injecting test double, or anything else without the
 * container knowing. Both functions receive @c context unchanged; containers
 * never inspect it.
 *
 * The allocator, and whatever @c context points at, must stay valid for as long
 * as any object built from it is alive. @c free must accept storage returned by
 * the matching @c alloc; containers never pass it nullptr.
 */
struct Allocator
{
    /**
     * Returns @p size bytes, or nullptr on failure. Receives @c context.
     *
     * The storage must be aligned for any object type, as malloc's is: Vector
     * places elements in it by byte offset and hands out typed pointers into
     * it, so an under-aligned block would be a misaligned element. Never
     * called with a @p size of 0.
     */
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
 * state. This is the allocator the plain @c *_new constructors use.
 *
 * @return An allocator that is always valid and requires no teardown.
 */
Allocator allocator_default();

#endif //GB_Z80_ASM_ALLOCATOR_H