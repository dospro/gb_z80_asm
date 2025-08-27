#include <stdarg.h>
#include <setjmp.h>
#include <string.h>
#include <cmocka.h>
#include "../src/string.h"
#include "string_iterator_tests.h"

#include <stdlib.h>

/**
 * Test StringIterator_next_line function returns the first line
 * of a text.
 *
 * The text has 3 lines and is loaded from a fake file
 *
 * @param state Used by CMocka
 */
void test_string_iterator_next_line(void** state)
{
    const char* fake_text = "First line\nSecond line\nThird line\n";
    const size_t fake_text_size = strlen(fake_text);
    FILE* fake_file = fmemopen((void*)fake_text, fake_text_size, "r");

    assert_non_null(fake_file);

    const StringBuffer buffer = StringBuffer_from_file(fake_file);
    fclose(fake_file);

    assert_true(StringBuffer_is_ok(&buffer));
    StringIterator iterator = StringBuffer_create_iterator(&buffer);
    const String result = StringIterator_next_line(&iterator);
    assert_int_equal(result.size, 10);
    assert_true(string_is_equal_cstr(result, "First line"));
}

/**
 * Test StringIterator_next_line function returns all lines
 *
 * The text has 3 lines and is loaded from a fake file
 * and validate that the three lines are read correctly
 *
 * @param state Used by CMocka
 */
void test_string_iterator_next_line_all(void** state)
{
    const char* fake_text = "First line\nSecond line\nThird line\n";
    const size_t fake_text_size = strlen(fake_text);
    FILE* fake_file = fmemopen((void*)fake_text, fake_text_size, "r");

    assert_non_null(fake_file);

    const StringBuffer buffer = StringBuffer_from_file(fake_file);
    fclose(fake_file);

    assert_true(StringBuffer_is_ok(&buffer));
    StringIterator iterator = StringBuffer_create_iterator(&buffer);
    String result[4];
    for (int i = 0; i < 4; i++)
    {
        result[i] = StringIterator_next_line(&iterator);
    }
    assert_true(string_is_equal_cstr(result[0], "First line"));
    assert_true(string_is_equal_cstr(result[1], "Second line"));
    assert_true(string_is_equal_cstr(result[2], "Third line"));
    assert_true(string_is_empty(result[3]));
}

/**
 * Test StringIterator_next_line function handles multiple newlines
 *
 * The test now uses a text which has multiple continuous new line
 * characters. The function must ignore those and just return the
 * next line of valid text.
 *
 * @param state Used by CMocka
 */
void test_string_iterator_next_line_ignore_multiple_new_lines(void** state)
{
    const char* fake_text = "\nFirst line\nSecond line\n\nThird line\n\n\n";
    const size_t fake_text_size = strlen(fake_text);
    FILE* fake_file = fmemopen((void*)fake_text, fake_text_size, "r");

    assert_non_null(fake_file);

    const StringBuffer buffer = StringBuffer_from_file(fake_file);
    fclose(fake_file);

    assert_true(StringBuffer_is_ok(&buffer));
    StringIterator iterator = StringBuffer_create_iterator(&buffer);
    String result[4];
    for (int i = 0; i < 4; i++)
    {
        result[i] = StringIterator_next_line(&iterator);
    }
    assert_true(string_is_equal_cstr(result[0], "First line"));
    assert_true(string_is_equal_cstr(result[1], "Second line"));
    assert_true(string_is_equal_cstr(result[2], "Third line"));
    assert_true(string_is_empty(result[3]));
}

/**
 * Test StringIterator_next_raw_line function return a string with
 * the newline character
 *
 * @param state Used by CMocka
 */
void test_string_iterator_next_raw_line_new_line(void** state)
{
    const StringBuffer buffer = StringBuffer_from_cstr("\n", malloc);
    StringIterator iterator = StringBuffer_create_iterator(&buffer);
    const String result = StringIterator_next_raw_line(&iterator);
    assert_true(string_is_equal_cstr(result, "\n"));
}

/**
 * Test StringIterator_next_raw_line function returns a line including
 * the newline character
 *
 * @param state Used by CMocka
 */
void test_string_iterator_next_raw_line_combined(void** state)
{
    const StringBuffer buffer = StringBuffer_from_cstr("First line\n", malloc);
    StringIterator iterator = StringBuffer_create_iterator(&buffer);
    const String result = StringIterator_next_raw_line(&iterator);
    assert_true(string_is_equal_cstr(result, "First line\n"));
}

/**
 * Test StringIterator_next_raw_line function with multiple lines
 *
 * @param state Used by CMocka
 */
void test_string_iterator_next_raw_line_multiple(void** state)
{
    const StringBuffer buffer = StringBuffer_from_cstr("\nFirst line\n\nOther line", malloc);
    StringIterator iterator = StringBuffer_create_iterator(&buffer);
    String result[5];
    for (int i = 0; i < 5; i++)
    {
        result[i] = StringIterator_next_raw_line(&iterator);
    }
    assert_true(string_is_equal_cstr(result[0], "\n"));
    assert_true(string_is_equal_cstr(result[1], "First line\n"));
    assert_true(string_is_equal_cstr(result[2], "\n"));
    assert_true(string_is_equal_cstr(result[3], "Other line"));
    assert_true(string_is_empty(result[4]));
}