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

/** Three-byte elements expose offset errors that padded structs can hide. */
typedef struct Triple
{
    uint8_t a;
    uint8_t b;
    uint8_t c;
} Triple;

static_assert(sizeof(Triple) == 3, "Triple must have no padding");

/**
 * Creation reserves storage without claiming any elements are initialized.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_new_reserves_storage(void** state)
{
    (void)state;
    Vector vector = Vector_new(sizeof(Triple), 4);

    assert_non_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 4);
    assert_int_equal(vector.element_size, sizeof(Triple));

    Vector_free(&vector);
}

/**
 * Creation must retain the supplied allocator and its per-instance context.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_new_uses_injected_allocator(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 2, allocator);

    assert_non_null(vector.data);
    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    assert_true(vector.allocator.alloc == controlled_alloc);
    assert_true(vector.allocator.free == controlled_free);
    assert_ptr_equal(vector.allocator.context, &control);

    Vector_free(&vector);
}

/**
 * Free releases reserved storage through its owner and resets the counts.
 * No insertion is needed to establish this lifecycle behavior.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_free_releases_storage(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 2, allocator);
    assert_non_null(vector.data);

    Vector_free(&vector);

    assert_int_equal(control.free_calls, 1);
    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.element_size, sizeof(Triple));
    assert_ptr_equal(vector.allocator.context, &control);
}

/**
 * Repeated free must not release the same allocation twice.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_free_is_idempotent(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 2, allocator);
    assert_non_null(vector.data);
    Vector_free(&vector);

    Vector_free(&vector);

    assert_int_equal(control.free_calls, 1);
    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
}

/**
 * Null cleanup is a separate no-crash contract, not a reuse scenario.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_free_accepts_null(void** state)
{
    (void)state;
    Vector_free(nullptr);
}

/**
 * Zero capacity is valid and must not depend on malloc(0) behavior.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_new_zero_capacity_skips_allocation(void** state)
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
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.element_size, sizeof(Triple));

    Vector_free(&vector);
    assert_int_equal(control.free_calls, 0);
}

/**
 * Failed reservation must not advertise storage that was never acquired.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_new_allocation_failure(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    control.fail_allocations = true;
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 2, allocator);

    assert_int_equal(control.alloc_calls, 1);
    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.element_size, sizeof(Triple));

    Vector_free(&vector);
    assert_int_equal(control.free_calls, 0);
}

/**
 * Zero-sized elements are unusable and must be rejected before allocation.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_new_rejects_zero_element_size(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(0, 2, allocator);

    assert_int_equal(control.alloc_calls, 0);
    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.element_size, 0);

    Vector_free(&vector);
    assert_int_equal(control.free_calls, 0);
}

/**
 * Capacity multiplication must be checked before calling the allocator.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_new_rejects_capacity_overflow(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), SIZE_MAX / sizeof(Triple) + 1, allocator);

    assert_int_equal(control.alloc_calls, 0);
    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(vector.element_size, sizeof(Triple));

    Vector_free(&vector);
    assert_int_equal(control.free_calls, 0);
}

/**
 * The first insertion establishes copying and size, independently of Vector_at.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_stores_one_element(void** state)
{
    (void)state;
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new(sizeof(Triple), 2);

    assert_true(Vector_push(&vector, &element));

    assert_int_equal(vector.size, 1);
    assert_int_equal(vector.capacity, 2);
    assert_memory_equal(vector.data, &element, sizeof(element));
    Vector_free(&vector);
}

/**
 * A second insertion establishes byte offsets and preservation of the first.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_places_second_element(void** state)
{
    (void)state;
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new(sizeof(Triple), 2);
    assert_true(Vector_push(&vector, &elements[0]));

    assert_true(Vector_push(&vector, &elements[1]));

    assert_int_equal(vector.size, 2);
    assert_int_equal(vector.capacity, 2);
    assert_memory_equal(vector.data, elements, 2 * sizeof(Triple));
    Vector_free(&vector);
}

/**
 * Insertion owns a byte copy, not a pointer to the caller's element.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_copies_source(void** state)
{
    (void)state;
    Triple source = {.a = 1, .b = 2, .c = 3};
    const Triple expected = source;
    Vector vector = Vector_new(sizeof(Triple), 1);
    assert_true(Vector_push(&vector, &source));

    source.a = 99;

    assert_memory_equal(vector.data, &expected, sizeof(expected));
    Vector_free(&vector);
}

/**
 * Access at zero must return the actual stored element.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_at_returns_first(void** state)
{
    (void)state;
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new(sizeof(Triple), 2);
    assert_true(Vector_push(&vector, &elements[0]));
    assert_true(Vector_push(&vector, &elements[1]));

    const Triple* const stored = Vector_at(&vector, 0);

    assert_non_null(stored);
    assert_ptr_equal(stored, (Triple*)vector.data + 0);
    assert_memory_equal(stored, &elements[0], sizeof(Triple));
    Vector_free(&vector);
}

/**
 * Access at a nonzero index must scale the offset by element_size.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_at_returns_second(void** state)
{
    (void)state;
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new(sizeof(Triple), 2);
    assert_true(Vector_push(&vector, &elements[0]));
    assert_true(Vector_push(&vector, &elements[1]));

    const Triple* const stored = Vector_at(&vector, 1);

    assert_non_null(stored);
    assert_ptr_equal(stored, (Triple*)vector.data + 1);
    assert_memory_equal(stored, &elements[1], sizeof(Triple));
    Vector_free(&vector);
}

/**
 * The first unused slot is inaccessible even with spare capacity.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_at_rejects_size(void** state)
{
    (void)state;
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new(sizeof(Triple), 4);
    assert_true(Vector_push(&vector, &element));

    assert_null(Vector_at(&vector, 1));

    Vector_free(&vector);
}

/**
 * An index at capacity is outside both initialized elements and storage.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_at_rejects_capacity(void** state)
{
    (void)state;
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new(sizeof(Triple), 4);
    assert_true(Vector_push(&vector, &element));

    assert_null(Vector_at(&vector, 4));

    Vector_free(&vector);
}

/**
 * An extreme index must be rejected before offset multiplication.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_at_rejects_maximum_index(void** state)
{
    (void)state;
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new(sizeof(Triple), 4);
    assert_true(Vector_push(&vector, &element));

    assert_null(Vector_at(&vector, SIZE_MAX));

    Vector_free(&vector);
}

/**
 * An empty vector has no readable element, even at index zero.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_at_rejects_empty(void** state)
{
    (void)state;
    Vector vector = Vector_new(sizeof(Triple), 0);

    assert_null(Vector_at(&vector, 0));

    Vector_free(&vector);
}

/**
 * A missing vector must not be dereferenced by the accessor.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_at_rejects_null(void** state)
{
    (void)state;
    assert_null(Vector_at(nullptr, 0));
}

/**
 * Reject null vector independently, before copying or allocating.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_rejects_null_vector(void** state)
{
    (void)state;
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    assert_false(Vector_push(nullptr, &element));
}

/**
 * Reject null source independently, before copying or allocating.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_rejects_null_source(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 0, allocator);

    assert_false(Vector_push(&vector, nullptr));

    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(control.alloc_calls, 0);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
}

/**
 * Reject zero element size independently, before copying or allocating.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_rejects_zero_element_size(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(0, 0, allocator);

    assert_false(Vector_push(&vector, &element));

    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(control.alloc_calls, 0);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
}

/**
 * First insertion into zero-capacity storage uses the floor of four,
 * without freeing a null allocation.
 *
 * @param state State supplied by CMocka; unused.
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
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 0, allocator);

    assert_true(Vector_push(&vector, &element));

    assert_int_equal(vector.size, 1);
    assert_int_equal(vector.capacity, 4);
    assert_memory_equal(vector.data, &element, sizeof(element));
    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
    assert_int_equal(control.free_calls, 1);
}

/**
 * Growth must retain the prefix and copy the new suffix in order.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_growth_preserves_contents(void** state)
{
    (void)state;
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new(sizeof(Triple), 2);
    assert_true(Vector_push(&vector, &elements[0]));
    assert_true(Vector_push(&vector, &elements[1]));

    assert_true(Vector_push(&vector, &elements[2]));

    assert_int_equal(vector.size, 3);
    assert_int_equal(vector.capacity, 4);
    assert_memory_equal(vector.data, elements, sizeof(elements));
    Vector_free(&vector);
}

/**
 * Growth allocates one replacement and frees the old block exactly once.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_growth_uses_allocator(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 1, allocator);
    assert_true(Vector_push(&vector, &elements[0]));

    assert_true(Vector_push(&vector, &elements[1]));

    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 1);
    assert_memory_equal(vector.data, elements, 2 * sizeof(Triple));
    Vector_free(&vector);
    assert_int_equal(control.free_calls, 2);
}

/**
 * Failed growth must preserve the pointer, counts and complete prefix.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_growth_failure_preserves_state(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 2, allocator);
    assert_true(Vector_push(&vector, &elements[0]));
    assert_true(Vector_push(&vector, &elements[1]));
    const void* const storage = vector.data;
    control.fail_allocations = true;

    assert_false(Vector_push(&vector, &elements[2]));

    assert_ptr_equal(vector.data, storage);
    assert_int_equal(vector.size, 2);
    assert_int_equal(vector.capacity, 2);
    assert_memory_equal(vector.data, elements, 2 * sizeof(Triple));
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
    assert_int_equal(control.free_calls, 1);
}

/**
 * Reuse is introduced only after insertion and growth from empty are established.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_free_leaves_vector_reusable(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 1, allocator);
    assert_true(Vector_push(&vector, &element));
    Vector_free(&vector);

    assert_true(Vector_push(&vector, &element));

    assert_int_equal(vector.size, 1);
    assert_int_equal(vector.capacity, 4);
    assert_memory_equal(vector.data, &element, sizeof(element));
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 1);
    Vector_free(&vector);
    assert_int_equal(control.free_calls, 2);
}

/**
 * A failed reservation retains enough allocator state for a later insertion.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_push_recovers_after_failed_reservation(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    control.fail_allocations = true;
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 2, allocator);
    assert_null(vector.data);
    control.fail_allocations = false;

    assert_true(Vector_push(&vector, &element));

    assert_int_equal(vector.size, 1);
    assert_memory_equal(vector.data, &element, sizeof(element));
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
    assert_int_equal(control.free_calls, 1);
}

/**
 * The first bulk append copies every byte and counts elements, not bytes.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_stores_range(void** state)
{
    (void)state;
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new(sizeof(Triple), 4);
    const void* const storage = vector.data;

    assert_true(Vector_append(&vector, elements, 3));

    assert_int_equal(vector.size, 3);
    assert_int_equal(vector.capacity, 4);
    assert_ptr_equal(vector.data, storage);
    assert_memory_equal(vector.data, elements, sizeof(elements));
    Vector_free(&vector);
}

/**
 * A second range must start at size rather than overwrite offset zero.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_places_second_range(void** state)
{
    (void)state;
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new(sizeof(Triple), 4);
    assert_true(Vector_append(&vector, elements, 1));
    const void* const storage = vector.data;

    assert_true(Vector_append(&vector, elements + 1, 2));

    assert_int_equal(vector.size, 3);
    assert_int_equal(vector.capacity, 4);
    assert_ptr_equal(vector.data, storage);
    assert_memory_equal(vector.data, elements, sizeof(elements));
    Vector_free(&vector);
}

/**
 * Reject null vector independently, before copying or allocating.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_rejects_null_vector(void** state)
{
    (void)state;
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    assert_false(Vector_append(nullptr, &element, 1));
}

/**
 * Reject null source independently, before copying or allocating.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_rejects_null_source(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 0, allocator);

    assert_false(Vector_append(&vector, nullptr, 1));

    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(control.alloc_calls, 0);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
}

/**
 * Reject zero element size independently, before copying or allocating.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_rejects_zero_element_size(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(0, 0, allocator);

    assert_false(Vector_append(&vector, &element, 1));

    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(control.alloc_calls, 0);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
}

/**
 * First insertion into zero-capacity storage uses the floor of four,
 * without freeing a null allocation.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_grows_from_empty(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 0, allocator);

    assert_true(Vector_append(&vector, &element, 1));

    assert_int_equal(vector.size, 1);
    assert_int_equal(vector.capacity, 4);
    assert_memory_equal(vector.data, &element, sizeof(element));
    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
    assert_int_equal(control.free_calls, 1);
}

/**
 * Growth must retain the prefix and copy the new suffix in order.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_growth_preserves_contents(void** state)
{
    (void)state;
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new(sizeof(Triple), 2);
    assert_true(Vector_push(&vector, &elements[0]));
    assert_true(Vector_push(&vector, &elements[1]));

    assert_true(Vector_append(&vector, &elements[2], 1));

    assert_int_equal(vector.size, 3);
    assert_int_equal(vector.capacity, 4);
    assert_memory_equal(vector.data, elements, sizeof(elements));
    Vector_free(&vector);
}

/**
 * Growth allocates one replacement and frees the old block exactly once.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_growth_uses_allocator(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 1, allocator);
    assert_true(Vector_push(&vector, &elements[0]));

    assert_true(Vector_append(&vector, &elements[1], 1));

    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 1);
    assert_memory_equal(vector.data, elements, 2 * sizeof(Triple));
    Vector_free(&vector);
    assert_int_equal(control.free_calls, 2);
}

/**
 * Failed growth must preserve the pointer, counts and complete prefix.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_growth_failure_preserves_state(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 2, allocator);
    assert_true(Vector_push(&vector, &elements[0]));
    assert_true(Vector_push(&vector, &elements[1]));
    const void* const storage = vector.data;
    control.fail_allocations = true;

    assert_false(Vector_append(&vector, &elements[2], 1));

    assert_ptr_equal(vector.data, storage);
    assert_int_equal(vector.size, 2);
    assert_int_equal(vector.capacity, 2);
    assert_memory_equal(vector.data, elements, 2 * sizeof(Triple));
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
    assert_int_equal(control.free_calls, 1);
}

/**
 * A bulk range larger than the growth heuristic must fit in one allocation.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_bulk_growth_exceeds_doubling(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple elements[3] = {
        {.a = 1, .b = 2, .c = 3},
        {.a = 4, .b = 5, .c = 6},
        {.a = 7, .b = 8, .c = 9},
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 1, allocator);

    assert_true(Vector_append(&vector, elements, 3));

    assert_int_equal(vector.size, 3);
    assert_int_equal(vector.capacity, 3);
    assert_memory_equal(vector.data, elements, sizeof(elements));
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 1);
    Vector_free(&vector);
}

/**
 * Growth from empty must jump straight past the floor of four to the required
 * size in one allocation.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_bulk_growth_exceeds_initial_floor(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple elements[] = {
        {.a = 1, .b = 2, .c = 3},
        {.a = 4, .b = 5, .c = 6},
        {.a = 7, .b = 8, .c = 9},
        {.a = 10, .b = 11, .c = 12},
        {.a = 13, .b = 14, .c = 15},
        {.a = 16, .b = 17, .c = 18},
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 0, allocator);

    assert_true(Vector_append(&vector, elements, 6));

    assert_int_equal(vector.size, 6);
    assert_int_equal(vector.capacity, 6);
    assert_memory_equal(vector.data, elements, sizeof(elements));
    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
}

/**
 * An empty range accepts a null source without changing an empty vector.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_zero_count_skips_allocation(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 0, allocator);
    const void* const storage = vector.data;

    assert_true(Vector_append(&vector, nullptr, 0));

    assert_ptr_equal(vector.data, storage);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(control.alloc_calls, 0);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
}

/**
 * An empty range accepts a null source without changing existing contents.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_zero_count_preserves_contents(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 0, allocator);
    assert_true(Vector_push(&vector, &element));
    const void* const storage = vector.data;

    assert_true(Vector_append(&vector, nullptr, 0));

    assert_ptr_equal(vector.data, storage);
    assert_int_equal(vector.size, 1);
    assert_int_equal(vector.capacity, 4);
    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    assert_memory_equal(vector.data, &element, sizeof(element));
    Vector_free(&vector);
}

/**
 * A real one-element vector plus SIZE_MAX elements overflows the count.
 * Reject before reading the deliberately undersized source range.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_rejects_count_overflow(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 1, allocator);
    assert_true(Vector_push(&vector, &element));
    const void* const storage = vector.data;

    assert_false(Vector_append(&vector, &element, SIZE_MAX));

    assert_ptr_equal(vector.data, storage);
    assert_int_equal(vector.size, 1);
    assert_int_equal(vector.capacity, 1);
    assert_memory_equal(vector.data, &element, sizeof(element));
    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
}

/**
 * The count fits size_t, but its three-byte representation does not.
 * Reject before allocation or reading the deliberately undersized source.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_rejects_byte_overflow(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple element = {.a = 0x11, .b = 0x22, .c = 0x33};
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 0, allocator);
    constexpr size_t count = SIZE_MAX / sizeof(Triple) + 1;

    assert_false(Vector_append(&vector, &element, count));

    assert_null(vector.data);
    assert_int_equal(vector.size, 0);
    assert_int_equal(vector.capacity, 0);
    assert_int_equal(control.alloc_calls, 0);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
}

/**
 * Appending an interior source range must preserve both prefix and suffix.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_own_subrange_without_growth(void** state)
{
    (void)state;
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new(sizeof(Triple), 6);
    assert_true(Vector_append(&vector, elements, 3));
    const Triple* const source = Vector_at(&vector, 1);

    assert_true(Vector_append(&vector, source, 2));

    assert_int_equal(vector.size, 5);
    assert_int_equal(vector.capacity, 6);
    assert_memory_equal(vector.data, elements, sizeof(elements));
    assert_memory_equal((Triple*)vector.data + 3, elements + 1, 2 * sizeof(Triple));
    Vector_free(&vector);
}

/**
 * Appending an interior source range must preserve both prefix and suffix,
 * even when growth releases the source allocation.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_own_subrange_with_growth(void** state)
{
    (void)state;
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new(sizeof(Triple), 3);
    assert_true(Vector_append(&vector, elements, 3));
    const Triple* const source = Vector_at(&vector, 1);

    assert_true(Vector_append(&vector, source, 2));

    assert_int_equal(vector.size, 5);
    assert_int_equal(vector.capacity, 6);
    assert_memory_equal(vector.data, elements, sizeof(elements));
    assert_memory_equal((Triple*)vector.data + 3, elements + 1, 2 * sizeof(Triple));
    Vector_free(&vector);
}

/**
 * A bulk append that cannot grow must not partially fill the remaining slot.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_failure_does_not_fill_spare_capacity(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 2, allocator);
    assert_true(Vector_push(&vector, &elements[0]));
    const void* const storage = vector.data;
    control.fail_allocations = true;

    assert_false(Vector_append(&vector, elements + 1, 2));

    assert_ptr_equal(vector.data, storage);
    assert_int_equal(vector.size, 1);
    assert_int_equal(vector.capacity, 2);
    assert_memory_equal(vector.data, elements, sizeof(Triple));
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 0);
    Vector_free(&vector);
}

/**
 * Required capacity includes existing elements, not just the new range.
 *
 * @param state State supplied by CMocka; unused.
 */
void test_vector_append_bulk_growth_preserves_prefix(void** state)
{
    (void)state;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };
    const Triple elements[] = {
        {.a = 0x11, .b = 0x22, .c = 0x33},
        {.a = 0x44, .b = 0x55, .c = 0x66},
        {.a = 0x77, .b = 0x88, .c = 0x99},
    };
    Vector vector = Vector_new_with_allocator(sizeof(Triple), 1, allocator);
    assert_true(Vector_push(&vector, &elements[0]));

    assert_true(Vector_append(&vector, elements + 1, 2));

    assert_int_equal(vector.size, 3);
    assert_int_equal(vector.capacity, 3);
    assert_memory_equal(vector.data, elements, sizeof(elements));
    assert_int_equal(control.alloc_calls, 2);
    assert_int_equal(control.free_calls, 1);
    Vector_free(&vector);
}
