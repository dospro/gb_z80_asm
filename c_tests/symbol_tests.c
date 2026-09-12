#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <cmocka.h>

#include "symbol_tests.h"
#include "../src/symbol.h"


/**
 * Records what the injected test allocator was asked to do.
 *
 * Allocator carries no user-data pointer, so the controlled functions below
 * reach this block through a file-scope pointer rather than a context
 * argument. Counting allocations and frees separately lets a test prove that
 * the table used the allocator it was given, and that it returned the storage
 * through the matching free.
 */
typedef struct AllocationControl AllocationControl;

struct AllocationControl
{
    size_t alloc_calls;
    size_t free_calls;
    bool fail_allocations;
};


static void* controlled_alloc(void* const context, const size_t size)
{
    AllocationControl* const control = context;
    control->alloc_calls++;
    return control->fail_allocations ? nullptr : malloc(size);
}

static void controlled_free(void* const context, void* const pointer)
{
    AllocationControl* const control = context;
    control->free_calls++;
    free(pointer);
}


/**
 * Tests that SymbolTable_new returns an empty table that already owns storage
 * for the requested number of symbols.
 *
 * Creation is the only behavior under test. The test asks for room for four
 * symbols, then verifies the three things a freshly created table promises:
 * it holds no symbols yet, it reports the capacity the caller asked for, and
 * it actually acquired backing storage. Freeing at the end keeps the
 * allocation balanced so a later failure here means creation broke, not
 * teardown.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_symbol_table_new_creates_empty_table(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 4;

    SymbolTable table = SymbolTable_new(requested_capacity);

    assert_int_equal(table.size, 0);
    assert_int_equal(table.capacity, requested_capacity);
    assert_non_null(table.symbols);

    SymbolTable_free(&table);
}

/**
 * Tests that SymbolTable_new refuses a capacity whose byte count would not fit
 * in a size_t, instead of reporting capacity it never allocated.
 *
 * The requested capacity is derived from sizeof(Symbol) rather than hardcoded,
 * so the test stays correct on any platform: SIZE_MAX / sizeof(Symbol) is the
 * largest capacity that still fits, and one more is guaranteed to overflow the
 * multiplication. The danger of an unchecked multiply is not a failed
 * allocation but a successful one -- the product wraps to a small number, the
 * allocator hands back a tiny block, and the table then advertises room for
 * quadrillions of symbols. The assertions therefore check that the table came
 * back empty and storage-free, which is the state a caller can safely detect.
 *
 * @param state State supplied by CMocka; unused by this test.
 */
void test_symbol_table_new_rejects_overflowing_capacity(void** state)
{
    (void)state;
    constexpr size_t overflowing_capacity = SIZE_MAX / sizeof(Symbol) + 1;

    SymbolTable table = SymbolTable_new(overflowing_capacity);

    assert_null(table.symbols);
    assert_int_equal(table.capacity, 0);
    assert_int_equal(table.size, 0);

    SymbolTable_free(&table);
}


/**
 * Tests that SymbolTable_new_with_allocator routes both the allocation and the
 * later release through the allocator it was handed.
 *
 * The plain constructor already covers the shape of a new table, so this test
 * concentrates on the part only injection can show. It builds a table with a
 * counting allocator and checks that exactly one allocation happened and no
 * free yet, which rules out a constructor that quietly falls back to malloc.
 * It then frees the table and checks the matching free arrived, proving the
 * allocator stored in the table survives the return and is what teardown uses.
 * That round trip is the invariant the allocation-failure tests will rely on.
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
    assert_int_equal(table.size, 0);
    assert_int_equal(table.capacity, requested_capacity);
    assert_non_null(table.symbols);

    SymbolTable_free(&table);

    assert_int_equal(control.free_calls, 1);
    assert_null(table.symbols);
}

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
    assert_int_equal(table.size, 1);
    assert_true(string_is_equal_cstr(table.symbols[0].name, "hello"));
    assert_int_equal(table.symbols[0].bank, 0);
    assert_int_equal(table.symbols[0].address, 0);
    assert_int_equal(table.symbols[0].definition_line, 1);

    SymbolTable_free(&table);
}

void test_symbol_table_add_symbol_reallocates(void** state)
{
    (void)state;
    constexpr size_t requested_capacity = 2;
    SymbolTable table = SymbolTable_new(requested_capacity);
    Symbol* symbols_storage = table.symbols;

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

    assert_int_equal(table.size, 3);
    assert_true(table.capacity > requested_capacity);
    assert_true(symbols_storage != table.symbols);
    assert_true(string_is_equal_cstr(table.symbols[0].name, "hello"));
    assert_int_equal(table.symbols[0].bank, 0);
    assert_true(string_is_equal_cstr(table.symbols[1].name, "world"));
    assert_int_equal(table.symbols[1].bank, 1);
    assert_true(string_is_equal_cstr(table.symbols[2].name, "new"));
    assert_int_equal(table.symbols[2].bank, 2);

    SymbolTable_free(&table);
}
