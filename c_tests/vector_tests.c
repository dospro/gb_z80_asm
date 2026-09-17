#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cmocka.h>

#include "vector_tests.h"
#include "allocator_double.h"
#include "../src/vector.h"


/**
 * Element type used throughout these tests.
 *
 * Three bytes wide on purpose: it is not a power of two and it is not padded
 * to any alignment, so an off-by-one in the byte arithmetic that places or
 * locates element N shows up as a corrupted neighbor rather than being hidden
 * by struct padding. The wrappers built on Vector only ever use aligned
 * struct sizes, so this is the one place that arithmetic is exercised with an
 * awkward size.
 */
typedef struct Triple Triple;

struct Triple
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
};

static_assert(sizeof(Triple) == 3, "Triple must be exactly three bytes to exercise unaligned element sizes");


/**
 * Tests that Vector_new returns an empty vector that already owns storage for
 * the requested number of elements and remembers their size.
 *
 * Creation is the only behavior under test. The test asks for room for four
 * three-byte elements, then verifies the four things a freshly created vector
 * promises: it holds no elements yet, it reports the capacity the caller asked
 * for, it recorded the element size it will use for every later copy, and it
 * actually acquired backing storage. Freeing at the end keeps the allocation
 * balanced so a later failure here means creation broke, not teardown.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_new_creates_empty_vector(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;

    Vector vector = Vector_new(sizeof(Triple), requested_capacity);

    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, requested_capacity);
    assert_int_equal(vector.element_size, sizeof(Triple));
    assert_non_null(vector.data);

    Vector_free(&vector);
}

/**
 * Tests that Vector_new refuses a capacity whose byte count would not fit in
 * a size_t, without ever asking the allocator for anything.
 *
 * The requested capacity is derived from the element size rather than
 * hardcoded, so the test stays correct on any platform: SIZE_MAX /
 * sizeof(Triple) is the largest capacity that still fits, and one more is
 * guaranteed to overflow the multiplication. The danger of an unchecked
 * multiply is not a failed allocation but a successful one -- the product
 * wraps to a small number, the allocator hands back a tiny block, and the
 * vector then advertises room for quadrillions of elements. A counting
 * allocator is injected so the test can assert directly that the allocator
 * was never called, rather than inferring it from the empty result; the
 * result is checked too, because that is the state a caller can detect.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_new_rejects_overflowing_capacity(void** state)
{
    (void)state;
    constexpr size_t overflowing_capacity = SIZE_MAX / sizeof(Triple) + 1;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };

    Vector vector = Vector_new_with_allocator(sizeof(Triple), overflowing_capacity, allocator);

    assert_int_equal(control.alloc_calls, 0);
    assert_null(vector.data);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.element_size, sizeof(Triple));

    Vector_free(&vector);

    assert_int_equal(control.free_calls, 0);
}

/**
 * Tests that Vector_new_with_allocator reports no capacity when the allocator
 * refuses the request, instead of advertising room it never got.
 *
 * The injected allocator is told to fail before the vector is built. The
 * danger here is a constructor that stores the requested capacity regardless
 * of whether the allocation succeeded: the first push would then skip growth
 * and write through a null data pointer. The assertions check that the
 * allocator was in fact asked once, that the vector came back with no storage
 * and zero capacity, and that freeing such a vector does not hand a null
 * pointer to the allocator's free.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_new_with_allocator_reports_no_capacity_on_failure(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    AllocationControl control = {.fail_allocations = true};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };

    Vector vector = Vector_new_with_allocator(sizeof(Triple), requested_capacity, allocator);

    assert_int_equal(control.alloc_calls, 1);
    assert_null(vector.data);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.element_size, sizeof(Triple));

    Vector_free(&vector);

    assert_int_equal(control.free_calls, 0);
}

/**
 * Tests that Vector_new refuses an element size of zero.
 *
 * A zero element size would make every later size computation degenerate:
 * the overflow guard divides by it, and push would copy nothing while still
 * advancing size. Rather than letting each operation defend against that,
 * creation rejects it up front. The assertions check the vector came back
 * empty, storage-free, and with the element size left at zero so the caller
 * can tell nothing was recorded.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_new_rejects_zero_element_size(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;

    Vector vector = Vector_new(0, requested_capacity);

    assert_null(vector.data);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.element_size, 0);

    Vector_free(&vector);
}

/**
 * Tests that Vector_new_with_allocator routes both the allocation and the
 * later release through the allocator it was handed.
 *
 * The plain constructor already covers the shape of a new vector, so this
 * test concentrates on the part only injection can show. It builds a vector
 * with a counting allocator and checks that exactly one allocation happened
 * and no free yet, which rules out a constructor that quietly falls back to
 * malloc. It then frees the vector and checks the matching free arrived,
 * proving the allocator stored in the vector survives the return and is what
 * teardown uses. That round trip is the invariant the allocation-failure
 * tests rely on.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_new_with_allocator_uses_injected_allocator(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };

    Vector vector = Vector_new_with_allocator(sizeof(Triple), requested_capacity, allocator);

    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, requested_capacity);
    assert_int_equal(vector.element_size, sizeof(Triple));
    assert_non_null(vector.data);

    Vector_free(&vector);

    assert_int_equal(control.free_calls, 1);
    assert_null(vector.data);
}

/**
 * Tests the three promises Vector_free makes beyond releasing storage: a
 * second call is harmless, a null vector is ignored, and the freed vector is
 * still usable.
 *
 * A vector is built with the counting allocator and given one element, then
 * freed. The first free must release exactly one block; a second free must
 * release nothing more, proving the pointer was cleared rather than freed
 * again. Freeing nullptr must not crash. Finally a push into the freed
 * vector must succeed with a fresh allocation and the original element size,
 * proving that free reset only the storage and counts, not the allocator or
 * the element size it needs to grow again.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_free_is_idempotent_and_leaves_vector_reusable(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 2;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), requested_capacity, allocator);
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    assert_true(Vector_push(&vector, &element));

    Vector_free(&vector);
    assert_int_equal(control.free_calls, 1);
    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.element_size, sizeof(Triple));

    Vector_free(&vector);
    assert_int_equal(control.free_calls, 1);

    Vector_free(nullptr);
    assert_int_equal(control.free_calls, 1);

    assert_true(Vector_push(&vector, &element));
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(vector.size, 1);
    const Triple* const stored = Vector_at(&vector, 0);
    assert_non_null(stored);
    assert_int_equal(stored->a, 0x11);
    assert_int_equal(stored->b, 0x22);
    assert_int_equal(stored->c, 0x33);

    Vector_free(&vector);
    assert_int_equal(control.free_calls, 2);
}

/**
 * Tests that Vector_push stores an element's bytes unchanged when there is
 * already room for it, and that Vector_at locates them again.
 *
 * Two elements are pushed into a vector with spare capacity, so the vector
 * never has to grow. Two rather than one, because the second element is the
 * first whose location depends on element_size being applied correctly: an
 * implementation that ignored element_size would still pass with a single
 * element at offset zero. The assertions check that each push reports
 * success, that size advances, and that every byte of both elements reads
 * back through Vector_at exactly as pushed. Reallocation behavior is covered
 * separately.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_push_default_case(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    Vector vector = Vector_new(sizeof(Triple), requested_capacity);

    const Triple first = {.a = 0x11, .b = 0x22, .c = 0x33};
    const Triple second = {.a = 0x44, .b = 0x55, .c = 0x66};

    assert_true(Vector_push(&vector, &first));
    assert_int_equal(vector.size, 1);
    assert_true(Vector_push(&vector, &second));
    assert_int_equal(vector.size, 2);
    assert_int_equal(vector.capacity, requested_capacity);

    const Triple* const stored_first = Vector_at(&vector, 0);
    assert_non_null(stored_first);
    assert_int_equal(stored_first->a, 0x11);
    assert_int_equal(stored_first->b, 0x22);
    assert_int_equal(stored_first->c, 0x33);

    const Triple* const stored_second = Vector_at(&vector, 1);
    assert_non_null(stored_second);
    assert_int_equal(stored_second->a, 0x44);
    assert_int_equal(stored_second->b, 0x55);
    assert_int_equal(stored_second->c, 0x66);

    Vector_free(&vector);
}

/**
 * Tests that Vector_push grows the backing storage once capacity is exceeded,
 * without losing or reordering the elements already stored.
 *
 * The vector starts with room for only two elements, and a third is pushed to
 * force growth. The assertions confirm growth actually happened -- capacity
 * increased and the storage pointer changed -- and that all three elements
 * are still readable afterward, byte for byte, in the order they were pushed.
 * The pointer comparison is sound because the new block is acquired before
 * the old one is released, so the two cannot share an address.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_push_reallocates(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 2;
    Vector vector = Vector_new(sizeof(Triple), requested_capacity);
    const void* const storage_before = vector.data;

    const Triple first = {.a = 0x11, .b = 0x22, .c = 0x33};
    const Triple second = {.a = 0x44, .b = 0x55, .c = 0x66};
    const Triple third = {.a = 0x77, .b = 0x88, .c = 0x99};

    assert_true(Vector_push(&vector, &first));
    assert_true(Vector_push(&vector, &second));
    assert_true(Vector_push(&vector, &third));

    assert_int_equal(vector.size, 3);
    assert_true(vector.capacity > requested_capacity);
    assert_ptr_not_equal(vector.data, storage_before);

    const Triple* const stored_first = Vector_at(&vector, 0);
    assert_int_equal(stored_first->a, 0x11);
    assert_int_equal(stored_first->b, 0x22);
    assert_int_equal(stored_first->c, 0x33);

    const Triple* const stored_second = Vector_at(&vector, 1);
    assert_int_equal(stored_second->a, 0x44);
    assert_int_equal(stored_second->b, 0x55);
    assert_int_equal(stored_second->c, 0x66);

    const Triple* const stored_third = Vector_at(&vector, 2);
    assert_int_equal(stored_third->a, 0x77);
    assert_int_equal(stored_third->b, 0x88);
    assert_int_equal(stored_third->c, 0x99);

    Vector_free(&vector);
}

/**
 * Tests that growing releases the old storage block through the allocator,
 * and releases it exactly once.
 *
 * The reallocation test above proves a new block was acquired but cannot see
 * whether the old one was leaked, because it goes through the C heap. This
 * test injects a counting allocator and forces one growth: it expects two
 * allocations (the initial block and the replacement) and one free (the
 * initial block) before teardown, then a second free for the replacement
 * after Vector_free. The elements are checked again afterwards so a free
 * issued too early -- before the copy -- would show up as corrupted data
 * rather than only as a miscount.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_push_growth_releases_old_storage(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 1;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), requested_capacity, allocator);

    const Triple first = {.a = 0x11, .b = 0x22, .c = 0x33};
    const Triple second = {.a = 0x44, .b = 0x55, .c = 0x66};

    assert_true(Vector_push(&vector, &first));
    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);

    assert_true(Vector_push(&vector, &second));
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 1);
    assert_int_equal(vector.size, 2);

    const Triple* const stored_first = Vector_at(&vector, 0);
    assert_int_equal(stored_first->a, 0x11);
    assert_int_equal(stored_first->b, 0x22);
    assert_int_equal(stored_first->c, 0x33);
    const Triple* const stored_second = Vector_at(&vector, 1);
    assert_int_equal(stored_second->a, 0x44);
    assert_int_equal(stored_second->b, 0x55);
    assert_int_equal(stored_second->c, 0x66);

    Vector_free(&vector);

    assert_int_equal(control.free_calls, 2);
}

/**
 * Tests that a failed growth in Vector_push leaves the vector exactly as it
 * was, with the element already stored still readable.
 *
 * A vector with room for one element is filled, then the allocator is told to
 * fail and a second element is pushed. The call must report failure without
 * touching the vector: size and capacity unchanged, the storage pointer
 * unchanged, the first element intact, and no free issued for storage that
 * is still in use. That last assertion pins the allocate-then-free ordering;
 * a realloc-style free-then-allocate would pass every other check and leave
 * the vector pointing at released memory.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_push_growth_failure_leaves_vector_intact(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 1;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), requested_capacity, allocator);
    const Triple first = {.a = 0x11, .b = 0x22, .c = 0x33};
    assert_true(Vector_push(&vector, &first));
    const void* const storage_before = vector.data;

    control.fail_allocations = true;
    const Triple second = {.a = 0x44, .b = 0x55, .c = 0x66};
    const bool result = Vector_push(&vector, &second);

    assert_false(result);
    assert_int_equal(vector.size, 1);
    assert_int_equal(vector.capacity, requested_capacity);
    assert_ptr_equal(vector.data, storage_before);
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 0);

    const Triple* const stored_first = Vector_at(&vector, 0);
    assert_non_null(stored_first);
    assert_int_equal(stored_first->a, 0x11);
    assert_int_equal(stored_first->b, 0x22);
    assert_int_equal(stored_first->c, 0x33);

    Vector_free(&vector);

    assert_int_equal(control.free_calls, 1);
}

/**
 * Tests that a vector created with capacity 0 holds no storage, and that the
 * first push then acquires some.
 *
 * Creation with capacity 0 must not touch the allocator: what malloc returns
 * for a zero-byte request is implementation-defined, and the vector should
 * not depend on it. So the test first checks no allocation happened and data
 * is null. The first push must then allocate a fresh block without copying
 * from or freeing the null pointer, neither of which the Allocator contract
 * allows. The assertions check that the push succeeds, that exactly one
 * allocation and no free occurred, that capacity jumped to the documented
 * initial size, and that the element reads back.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_push_grows_from_empty(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 0, allocator);
    assert_int_equal(control.alloc_calls, 0);
    assert_null(vector.data);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.element_size, sizeof(Triple));

    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    const bool result = Vector_push(&vector, &element);

    assert_true(result);
    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    assert_int_equal(vector.size, 1);
    assert_int_equal(vector.capacity, 4);
    assert_non_null(vector.data);

    const Triple* const stored = Vector_at(&vector, 0);
    assert_non_null(stored);
    assert_int_equal(stored->a, 0x11);
    assert_int_equal(stored->b, 0x22);
    assert_int_equal(stored->c, 0x33);

    Vector_free(&vector);

    assert_int_equal(control.free_calls, 1);
}

/**
 * Tests that Vector_push reports failure, and changes nothing, when handed a
 * null vector or a null element.
 *
 * Both null cases are checked in one test because they share a contract:
 * return false and touch nothing. For the null element case the vector is a
 * real one with spare capacity, so a push that ignored the element pointer
 * and advanced size anyway would be caught by the size assertion.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_push_rejects_null_arguments(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    Vector vector = Vector_new(sizeof(Triple), requested_capacity);
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};

    assert_false(Vector_push(nullptr, &element));

    assert_false(Vector_push(&vector, nullptr));
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, requested_capacity);

    Vector_free(&vector);
}

/**
 * Tests that Vector_at returns nullptr for any index at or past size, even
 * when that index is still within capacity.
 *
 * A vector with room for four elements holds only one. Index 1 is inside the
 * allocated block but has never been written, so handing it out would expose
 * uninitialized bytes; the vector must treat size, not capacity, as the
 * bound. The test also checks index 0 is still served, so the bound is
 * exactly size and not off by one, and that a null vector is rejected.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_vector_at_rejects_out_of_range_index(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    Vector vector = Vector_new(sizeof(Triple), requested_capacity);
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    assert_true(Vector_push(&vector, &element));

    assert_non_null(Vector_at(&vector, 0));
    assert_null(Vector_at(&vector, 1));
    assert_null(Vector_at(&vector, requested_capacity));
    assert_null(Vector_at(&vector, SIZE_MAX));
    assert_null(Vector_at(nullptr, 0));

    Vector_free(&vector);
}
