#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <cmocka.h>

#include "fixups_tests.h"
#include "allocator_double.h"
#include "../src/fixups.h"


/**
 * Tests that FixupTable_new returns an empty table whose backing vector is
 * sized for Fixup and already owns storage for the requested count.
 *
 * Creation is the only behavior under test. The test asks for room for four
 * fixups, then verifies what a freshly created table promises: it holds no
 * fixups yet, it reports the capacity the caller asked for, the vector was
 * told the element size is sizeof(Fixup), and it actually acquired backing
 * storage. The element-size check is the one thing the wrapper adds over
 * Vector, so it is asserted here rather than left to the vector tests.
 * Freeing at the end keeps the allocation balanced so a later failure here
 * means creation broke, not teardown.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_fixups_table_new_creates_empty_table(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;

    FixupTable table = FixupTable_new(requested_capacity);

    assert_int_equal(table.fixups.size, 0);
    assert_int_equal(table.fixups.capacity, requested_capacity);
    assert_int_equal(table.fixups.element_size, sizeof(Fixup));
    assert_non_null(table.fixups.data);

    FixupTable_free(&table);
}

/**
 * Tests that FixupTable_new refuses a capacity whose byte count would not fit
 * in a size_t, instead of reporting capacity it never allocated.
 *
 * The requested capacity is derived from sizeof(Fixup) rather than hardcoded,
 * so the test stays correct on any platform: SIZE_MAX / sizeof(Fixup) is the
 * largest capacity that still fits, and one more is guaranteed to overflow the
 * multiplication. The overflow guard itself lives in Vector and is tested
 * there; this test proves the wrapper passes sizeof(Fixup) through so the
 * guard is computed against the right element size.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_fixups_table_new_rejects_overflowing_capacity(void** state)
{
    (void)state;
    constexpr size_t overflowing_capacity = SIZE_MAX / sizeof(Fixup) + 1;

    FixupTable table = FixupTable_new(overflowing_capacity);

    assert_null(table.fixups.data);
    assert_int_equal(table.fixups.capacity, 0);
    assert_int_equal(table.fixups.size, 0);

    FixupTable_free(&table);
}


/**
 * Tests that FixupTable_new_with_allocator routes both the allocation and the
 * later release through the allocator it was handed.
 *
 * The plain constructor already covers the shape of a new table, so this test
 * concentrates on the part only injection can show. It builds a table with a
 * counting allocator and checks that exactly one allocation happened and no
 * free yet, which rules out a wrapper that quietly substitutes the default
 * allocator on its way into Vector. It then frees the table and checks the
 * matching free arrived.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_fixups_table_new_with_allocator_uses_injected_allocator(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };

    FixupTable table = FixupTable_new_with_allocator(
        requested_capacity,
        allocator
    );

    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    assert_int_equal(table.fixups.size, 0);
    assert_int_equal(table.fixups.capacity, requested_capacity);
    assert_non_null(table.fixups.data);

    FixupTable_free(&table);

    assert_int_equal(control.free_calls, 1);
    assert_null(table.fixups.data);
}

/**
 * Tests that FixupTable_add_fixup stores a fixup's fields unchanged when
 * there is already room for it, and that FixupTable_at returns them typed.
 *
 * A single fixup is added to a table with spare capacity, so the table never
 * has to grow. The assertions check that the call reports success, that size
 * advances by one, and that every field of the stored fixup -- the label
 * text, bank, placeholder address, source line, and kind -- matches what
 * was passed in. Reading the fields back through FixupTable_at is the point:
 * it proves the wrapper's typed accessor lands on the same bytes Vector
 * stored.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_fixups_table_add_fixup_default_case(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    FixupTable table = FixupTable_new(requested_capacity);

    const Fixup fixup = {
        .label = string_from_cstr("hello"),
        .bank = 0,
        .address = 0x150,
        .line = 1,
        .kind = FIXUP_ABSOLUTE_16,
    };

    const bool result = FixupTable_add_fixup(&table, fixup);
    assert_true(result);
    assert_int_equal(table.fixups.size, 1);

    const Fixup* const stored = FixupTable_at(&table, 0);
    assert_non_null(stored);
    assert_true(string_is_equal_cstr(stored->label, "hello"));
    assert_int_equal(stored->bank, 0);
    assert_int_equal(stored->address, 0x150);
    assert_int_equal(stored->line, 1);
    assert_int_equal(stored->kind, FIXUP_ABSOLUTE_16);

    FixupTable_free(&table);
}

/**
 * Tests that FixupTable_add_fixup grows the backing storage once capacity is
 * exceeded, without losing or reordering the fixups already stored.
 *
 * The table starts with room for only two fixups, and a third is added to
 * force growth. The assertions confirm growth actually happened -- capacity
 * increased and the storage pointer changed -- and that all three fixups are
 * still readable afterward, in the order they were added. The growth
 * mechanics belong to Vector; this test shows they hold when the element is
 * a Fixup, which is larger and pointer-aligned unlike the vector tests'
 * element.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_fixups_table_add_fixup_reallocates(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 2;
    FixupTable table = FixupTable_new(requested_capacity);
    const void* const storage_before = table.fixups.data;

    const Fixup fixup1 = {
        .label = string_from_cstr("hello"),
        .bank = 0,
        .address = 0x150,
        .line = 1,
        .kind = FIXUP_ABSOLUTE_16,
    };
    const Fixup fixup2 = {
        .label = string_from_cstr("world"),
        .bank = 1,
        .address = 0x151,
        .line = 2,
        .kind = FIXUP_RELATIVE_8,
    };
    const Fixup fixup3 = {
        .label = string_from_cstr("new"),
        .bank = 2,
        .address = 0x152,
        .line = 3,
        .kind = FIXUP_ABSOLUTE_16,
    };

    bool result = FixupTable_add_fixup(&table, fixup1);
    assert_true(result);
    result = FixupTable_add_fixup(&table, fixup2);
    assert_true(result);
    result = FixupTable_add_fixup(&table, fixup3);
    assert_true(result);

    assert_int_equal(table.fixups.size, 3);
    assert_true(table.fixups.capacity > requested_capacity);
    assert_ptr_not_equal(table.fixups.data, storage_before);
    assert_true(string_is_equal_cstr(FixupTable_at(&table, 0)->label, "hello"));
    assert_int_equal(FixupTable_at(&table, 0)->bank, 0);
    assert_true(string_is_equal_cstr(FixupTable_at(&table, 1)->label, "world"));
    assert_int_equal(FixupTable_at(&table, 1)->bank, 1);
    assert_true(string_is_equal_cstr(FixupTable_at(&table, 2)->label, "new"));
    assert_int_equal(FixupTable_at(&table, 2)->bank, 2);

    FixupTable_free(&table);
}

/**
 * Tests that FixupTable_at returns nullptr past the last stored fixup and for
 * a null table.
 *
 * The bounds check itself is Vector's; this test proves the wrapper forwards
 * the index unchanged rather than, say, scaling it by sizeof(Fixup) a second
 * time. A table holding one fixup must serve index 0 and refuse index 1, even
 * though index 1 is within the allocated capacity.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_fixups_table_at_rejects_out_of_range_index(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    FixupTable table = FixupTable_new(requested_capacity);
    const Fixup fixup = {
        .label = string_from_cstr("hello"),
        .bank = 0,
        .address = 0x150,
        .line = 1,
        .kind = FIXUP_ABSOLUTE_16,
    };
    assert_true(FixupTable_add_fixup(&table, fixup));

    assert_non_null(FixupTable_at(&table, 0));
    assert_null(FixupTable_at(&table, 1));
    assert_null(FixupTable_at(&table, requested_capacity));
    assert_null(FixupTable_at(nullptr, 0));

    FixupTable_free(&table);
}
