#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <cmocka.h>

#include "symbol_tests.h"
#include "allocator_double.h"
#include "../src/symbol.h"


/**
 * Tests that SymbolTable_new returns an empty table whose backing vector is
 * sized for Symbol and already owns storage for the requested count.
 *
 * Creation is the only behavior under test. The test asks for room for four
 * symbols, then verifies what a freshly created table promises: it holds no
 * symbols yet, it reports the capacity the caller asked for, the vector was
 * told the element size is sizeof(Symbol), and it actually acquired backing
 * storage. The element-size check is the one thing the wrapper adds over
 * Vector, so it is asserted here rather than left to the vector tests.
 * Freeing at the end keeps the allocation balanced so a later failure here
 * means creation broke, not teardown.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_symbol_table_new_creates_empty_table(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;

    SymbolTable table = SymbolTable_new(requested_capacity);

    assert_int_equal(table.symbols.size, 0);
    assert_int_equal(table.symbols.capacity, requested_capacity);
    assert_int_equal(table.symbols.element_size, sizeof(Symbol));
    assert_non_null(table.symbols.data);

    SymbolTable_free(&table);
}

/**
 * Tests that SymbolTable_new refuses a capacity whose byte count would not fit
 * in a size_t, instead of reporting capacity it never allocated.
 *
 * The requested capacity is derived from sizeof(Symbol) rather than hardcoded,
 * so the test stays correct on any platform: SIZE_MAX / sizeof(Symbol) is the
 * largest capacity that still fits, and one more is guaranteed to overflow the
 * multiplication. The overflow guard itself lives in Vector and is tested
 * there; this test proves the wrapper passes sizeof(Symbol) through so the
 * guard is computed against the right element size.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_symbol_table_new_rejects_overflowing_capacity(void** state)
{
    (void)state;
    constexpr size_t overflowing_capacity = SIZE_MAX / sizeof(Symbol) + 1;

    SymbolTable table = SymbolTable_new(overflowing_capacity);

    assert_null(table.symbols.data);
    assert_int_equal(table.symbols.capacity, 0);
    assert_int_equal(table.symbols.size, 0);

    SymbolTable_free(&table);
}


/**
 * Tests that SymbolTable_new_with_allocator routes both the allocation and the
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
void test_symbol_table_new_with_allocator_uses_injected_allocator(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    AllocationControl control = {};
    const Allocator allocator = {
        .alloc = controlled_alloc,
        .free = controlled_free,
        .context = &control,
    };

    SymbolTable table = SymbolTable_new_with_allocator(
        requested_capacity,
        allocator
    );

    assert_int_equal(control.alloc_calls, 1);
    assert_int_equal(control.free_calls, 0);
    assert_int_equal(table.symbols.size, 0);
    assert_int_equal(table.symbols.capacity, requested_capacity);
    assert_non_null(table.symbols.data);

    SymbolTable_free(&table);

    assert_int_equal(control.free_calls, 1);
    assert_null(table.symbols.data);
}

/**
 * Tests that SymbolTable_add_symbol stores a symbol's fields unchanged when
 * there is already room for it, and that SymbolTable_at returns them typed.
 *
 * A single symbol is added to a table with spare capacity, so the table never
 * has to grow. The assertions check that the call reports success, that size
 * advances by one, and that every field of the stored symbol -- the name
 * text, bank, address, and definition line -- matches what was passed in.
 * Reading the fields back through SymbolTable_at is the point: it proves the
 * wrapper's typed accessor lands on the same bytes Vector stored.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_symbol_table_add_symbol_default_case(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    SymbolTable table = SymbolTable_new(requested_capacity);

    const Symbol symbol = {
        .name = string_from_cstr("hello"),
        .bank = 0,
        .address = 0,
        .definition_line = 1,
    };

    const bool result = SymbolTable_add_symbol(&table, symbol);
    assert_true(result);
    assert_int_equal(table.symbols.size, 1);

    const Symbol* const stored = SymbolTable_at(&table, 0);
    assert_non_null(stored);
    assert_true(string_is_equal_cstr(stored->name, "hello"));
    assert_int_equal(stored->bank, 0);
    assert_int_equal(stored->address, 0);
    assert_int_equal(stored->definition_line, 1);

    SymbolTable_free(&table);
}

/**
 * Tests that SymbolTable_add_symbol grows the backing storage once capacity
 * is exceeded, without losing or reordering the symbols already stored.
 *
 * The table starts with room for only two symbols, and a third is added to
 * force growth. The assertions confirm growth actually happened -- capacity
 * increased and the storage pointer changed -- and that all three symbols are
 * still readable afterward, in the order they were added. The growth
 * mechanics belong to Vector; this test shows they hold when the element is
 * a Symbol, which is larger and pointer-aligned unlike the vector tests'
 * element.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_symbol_table_add_symbol_reallocates(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 2;
    SymbolTable table = SymbolTable_new(requested_capacity);
    const void* const storage_before = table.symbols.data;

    const Symbol symbol1 = {
        .name = string_from_cstr("hello"),
        .bank = 0,
        .address = 0x150,
        .definition_line = 1,
    };
    const Symbol symbol2 = {
        .name = string_from_cstr("world"),
        .bank = 1,
        .address = 0x151,
        .definition_line = 2,
    };
    const Symbol symbol3 = {
        .name = string_from_cstr("new"),
        .bank = 2,
        .address = 0x152,
        .definition_line = 3,
    };

    bool result = SymbolTable_add_symbol(&table, symbol1);
    assert_true(result);
    result = SymbolTable_add_symbol(&table, symbol2);
    assert_true(result);
    result = SymbolTable_add_symbol(&table, symbol3);
    assert_true(result);

    assert_int_equal(table.symbols.size, 3);
    assert_true(table.symbols.capacity > requested_capacity);
    assert_ptr_not_equal(table.symbols.data, storage_before);
    assert_true(string_is_equal_cstr(SymbolTable_at(&table, 0)->name, "hello"));
    assert_int_equal(SymbolTable_at(&table, 0)->bank, 0);
    assert_true(string_is_equal_cstr(SymbolTable_at(&table, 1)->name, "world"));
    assert_int_equal(SymbolTable_at(&table, 1)->bank, 1);
    assert_true(string_is_equal_cstr(SymbolTable_at(&table, 2)->name, "new"));
    assert_int_equal(SymbolTable_at(&table, 2)->bank, 2);

    SymbolTable_free(&table);
}

/**
 * Tests that SymbolTable_at returns nullptr past the last stored symbol and
 * for a null table.
 *
 * The bounds check itself is Vector's; this test proves the wrapper forwards
 * the index unchanged rather than, say, scaling it by sizeof(Symbol) a second
 * time. A table holding one symbol must serve index 0 and refuse index 1,
 * even though index 1 is within the allocated capacity.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_symbol_table_at_rejects_out_of_range_index(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;
    SymbolTable table = SymbolTable_new(requested_capacity);
    const Symbol symbol = {
        .name = string_from_cstr("hello"),
        .bank = 0,
        .address = 0x150,
        .definition_line = 1,
    };
    assert_true(SymbolTable_add_symbol(&table, symbol));

    assert_non_null(SymbolTable_at(&table, 0));
    assert_null(SymbolTable_at(&table, 1));
    assert_null(SymbolTable_at(&table, requested_capacity));
    assert_null(SymbolTable_at(nullptr, 0));

    SymbolTable_free(&table);
}
