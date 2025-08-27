#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>
#include "../src/string.h"
#include "string_tests.h"

/**
 * Test string_trim function removes spaces from beginning and end of a string.
 * @param state Used by CMocka
 */
void test_string_trim(void** state)
{
    const String info = {.data = "   test string  ", .size = 16};
    const String expected = {.data = "test string", .size = 11};

    const String result = string_trim(info);
    assert_true(string_is_equal(result, expected));
}

/**
 * Test string_trim function removes spaces from beginning of
 * a string.
 * @param state Used by CMocka
 */
void test_string_trim_beginning(void** state)
{
    const String info = {.data = "   test string\n", .size = strlen("   test string\n")};
    const String expected = {.data = "test string", .size = strlen("test string")};

    const String result = string_trim(info);
    assert_true(string_is_equal(result, expected));
}

/**
 * Test string_trim function removes spaces from the end of
 * a string.
 * @param state Used by CMocka
 */
void test_string_trim_end(void** state)
{
    const String info = {.data = "test string  \n", .size = strlen("test string  \n")};
    const String expected = {.data = "test string", .size = strlen("test string")};

    const String result = string_trim(info);
    assert_true(string_is_equal(result, expected));
}

/**
 * Test string_at functions returns the first character of a string when
 * index is zero
 * @param state Used by CMocka
 */
void test_string_at_zero(void** state)
{
    const String info = string_from_cstr("test");

    const char result = string_at(info, 0);
    assert_int_equal(result, 't');
}

/**
 * Test string_at functions returns an arbitrary character
 * depending on the index
 *
 * The test will use 2 indices.
 * @param state Used by CMocka
 */
void test_string_at_indexes(void** state)
{
    const String info = string_from_cstr("test");

    assert_int_equal(string_at(info, 1), 'e');
    assert_int_equal(string_at(info, 3), 't');
}

/**
 * Test string_at functions returns -1 when the index is invalid
 *
 * @param state Used by CMocka
 */
void test_string_at_invalid(void** state)
{
    const String info = string_from_cstr("test");

    assert_int_equal(string_at(info, 7), -1);
}
