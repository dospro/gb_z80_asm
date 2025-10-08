#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>
#include "../src/string.h"
#include "string_tests.h"

#include <stdlib.h>

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

/**
 * Test string_contains_char functions false if the string
 * doesn't contain a specific character
 *
 * @param state Used by CMocka
 */
void test_string_contains_char_false(void** state)
{
    const String info = string_from_cstr("test");
    assert_false(string_contains_char(info, 'c'));
}

/**
 * Test string_contains_char functions true if the string
 * contain a specific character
 *
 * @param state Used by CMocka
 */
void test_string_contains_char_true(void** state)
{
    const String info = string_from_cstr("test");
    assert_true(string_contains_char(info, 's'));
}

/**
 * Test string_from_string_buffer returns a String with the contents of the
 * StringBuffer.
 *
 * @param state Used by CMocka
 */
void test_string_from_string_buffer(void** state)
{
    const StringBuffer buffer = StringBuffer_from_cstr("Hello", malloc);
    const String result = string_from_string_buffer(buffer);
    assert_true(string_is_equal_cstr(result, "Hello"));
}

/**
 * When getting a String from a StringBuffer, the buffer cannot be further modified
 *
 * @param state Used by CMocka
 */
void test_string_from_string_buffer_freezes_buffer(void** state)
{
    StringBuffer buffer = StringBuffer_from_cstr("Hello", malloc);
    const String result = string_from_string_buffer(buffer);
    StringBuffer_append_cstr(&buffer, " world");
    assert_true(string_is_equal_cstr(result, "Hello"));
}