#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>

#include "string_buffer_tests.h"

#include <string.h>

#include "../src/string.h"

void* bad_malloc(size_t __size)
{
    return NULL;
}


/**
 * Test creation of a StringBuffer.
 * The test will create a String Buffer with a small capacity
 * and validate all internal values are set correctly
 *
 * @param state Used by CMocka
 */
void test_string_buffer_new(void** state)
{
    const StringBuffer buffer = StringBuffer_new(100, malloc);
    assert_int_equal(buffer.capacity, 100);
    assert_int_equal(buffer.size, 0);
    assert_non_null(buffer.data);
}

/**
 * Test StringBuffer_is_ok function returns true when buffer is created successfully
 * @param state Used by CMocka
 */
void test_string_buffer_is_ok(void** state)
{
    const StringBuffer buffer = StringBuffer_new(100, malloc);
    bool result = StringBuffer_is_ok(&buffer);
    assert_true(result);
}

/**
 * Test StringBuffer_is_ok function returns false when creation fails
 * The test uses a mocked malloc function called bad_malloc which always return null
 * @param state Used by CMocka
 */
void test_string_buffer_new_error(void** state)
{
    const StringBuffer buffer = StringBuffer_new(100, bad_malloc);
    auto result = StringBuffer_is_ok(&buffer);
    assert_false(result);
}

/**
 * Test StringBuffer_from_file function reads an entire text file
 * The test uses a mocked file which contains a line of text.
 *
 * In this case we use fmemopen function which will help us fake
 * a file using a string.
 * @param state Used by CMocka
 */
void test_string_buffer_read_file(void** state)
{
    const char* fake_text = "Hello world!\n";
    const size_t fake_text_size = strlen(fake_text);
    FILE* fake_file = fmemopen((void*)fake_text, fake_text_size, "r");

    assert_non_null(fake_file);

    const StringBuffer result = StringBuffer_from_file(fake_file);
    fclose(fake_file);

    assert_true(StringBuffer_is_ok(&result));
    assert_int_equal(result.size, fake_text_size);
    assert_int_equal(result.capacity, fake_text_size);
}

/**
 * Test StringBuffer_from_file function returns a null buffer
 * which can be validated with StringBuffer_is_ok when it fails
 * to read a file
 *
 * In this case FILE has a NULL value
 * @param state Used by CMocka
 */
void test_string_buffer_read_file_fails(void** state)
{
    FILE* fake_file = nullptr;

    const StringBuffer result = StringBuffer_from_file(fake_file);

    assert_false(StringBuffer_is_ok(&result));
}

/**
 * Test StringBuffer_create_iterator function returns a StringIterator
 * with the correct values
 *
 * @param state Used by CMocka
 */
void test_string_buffer_to_iterator(void** state)
{
    const char* fake_text = "Hello world!\n";
    const size_t fake_text_size = strlen(fake_text);
    FILE* fake_file = fmemopen((void*)fake_text, fake_text_size, "r");

    assert_non_null(fake_file);

    const StringBuffer buffer = StringBuffer_from_file(fake_file);
    fclose(fake_file);
    const StringIterator iterator = StringBuffer_create_iterator(&buffer);
    assert_int_equal(iterator.source.data, buffer.data);
    assert_int_equal(iterator.source.size, fake_text_size);
    assert_int_equal(iterator.position, 0);
}

/**
 * Test StringBuffer_create_iterator function returns an invalid
 * iterator instead of crashing when a bad buffer is passed
 *
 * @param state Used by CMocka
 */
void test_string_buffer_to_iterator_bad_buffer(void** state)
{
    const StringBuffer buffer = {nullptr};
    const StringIterator iterator = StringBuffer_create_iterator(&buffer);
    assert_false(StringIterator_is_ok(&iterator));
}

/**
 * Test StringBuffer_append_cstr appends strings in a buffer of
 * enough capacity.
 *
 * @param state Used by CMocka
 */
void test_string_buffer_append_c_string(void** state)
{
    StringBuffer buffer = StringBuffer_new(200, malloc);
    StringBuffer_append_cstr(&buffer,"Hello");
    StringBuffer_append_cstr(&buffer, " world");
    assert_true(StringBuffer_is_ok(&buffer));
    const String result = string_from_string_buffer(buffer);
    assert_true(string_is_equal_cstr(result, "Hello world"));
}

/**
 * Test StringBuffer_append_cstr appends strings in a buffer of
 * with no capacity.
 *
 * The function should reallocate the memory without any side effect.
 *
 * @param state Used by CMocka
 */
void test_string_buffer_append_c_string_no_capacity(void** state)
{
    StringBuffer buffer = StringBuffer_new(5, malloc);
    StringBuffer_append_cstr(&buffer,"Hello");
    StringBuffer_append_cstr(&buffer, " world");
    assert_true(StringBuffer_is_ok(&buffer));
    const String result = string_from_string_buffer(buffer);
    assert_true(string_is_equal_cstr(result, "Hello world"));
}