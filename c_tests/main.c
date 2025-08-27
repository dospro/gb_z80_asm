#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#include "../src/string.h"
#include "string_tests.h"
#include "string_buffer_tests.h"
#include "string_iterator_tests.h"
#include "opcode_tests.h"


// void test_string_split(void** state)
// {
//     const char* code = "add a,b";
//     const String info = {.data = code, .size = strlen(code)};
//     const String expected[] = {
//         {.data = "add", .size = 3},
//         {.data = "a", .size = 1},
//         {.data = "b", .size = 1}
//     };
//     StringIterator iterator = StringIterator_new(info);
//     const String first = StringIterator_next(&iterator);
//     const String second = StringIterator_next(&iterator);
//     const String third = StringIterator_next(&iterator);
//     const String fourth = StringIterator_next(&iterator);
//     assert_int_equal(first.size, expected[0].size);
//     assert_int_equal(second.size, expected[1].size);
//     assert_int_equal(third.size, expected[2].size);
//     assert_int_equal(fourth.size, 0);
//
//     for (size_t i = 0; i < first.size; i++)
//     {
//         assert_true(first.data[i] == expected[0].data[i]);
//     }
//
//     for (size_t i = 0; i < second.size; i++)
//     {
//         assert_true(second.data[i] == expected[1].data[i]);
//     }
//
//     for (size_t i = 0; i < third.size; i++)
//     {
//         assert_true(third.data[i] == expected[2].data[i]);
//     }
// }
//
//
// void test_string_split_function(void** state)
// {
//     // Test case 1: Basic split with spaces
//     {
//         const char* text = "hello world test";
//         const char* delimiters = " ";
//         const String str = {.data = (char*)text, .size = strlen(text)};
//         const String pattern = {.data = (char*)delimiters, .size = strlen(delimiters)};
//
//         Vector result = string_split(str, pattern);
//         String* strings = (String*)result.data;
//
//         // Verify we have 3 substrings
//         assert_int_equal(result.size, 3);
//
//         // Verify each substring
//         assert_int_equal(strings[0].size, 5); // "hello"
//         assert_memory_equal(strings[0].data, "hello", 5);
//
//         assert_int_equal(strings[1].size, 5); // "world"
//         assert_memory_equal(strings[1].data, "world", 5);
//
//         assert_int_equal(strings[2].size, 4); // "test"
//         assert_memory_equal(strings[2].data, "test", 4);
//
//         // Free allocated memory
//         free(result.data);
//     }
//
//     // Test case 2: Multiple delimiters
//     {
//         const char* text = "a,b;c:d";
//         const char* delimiters = ",;:";
//         const String str = {.data = (char*)text, .size = strlen(text)};
//         const String pattern = {.data = (char*)delimiters, .size = strlen(delimiters)};
//
//         Vector result = string_split(str, pattern);
//         String* strings = (String*)result.data;
//
//         // Verify we have 4 substrings
//         assert_int_equal(result.size, 4);
//
//         // Verify each substring
//         assert_int_equal(strings[0].size, 1); // "a"
//         assert_memory_equal(strings[0].data, "a", 1);
//
//         assert_int_equal(strings[1].size, 1); // "b"
//         assert_memory_equal(strings[1].data, "b", 1);
//
//         assert_int_equal(strings[2].size, 1); // "c"
//         assert_memory_equal(strings[2].data, "c", 1);
//
//         assert_int_equal(strings[3].size, 1); // "d"
//         assert_memory_equal(strings[3].data, "d", 1);
//
//         // Free allocated memory
//         free(result.data);
//     }
//
//     // Test case 3: Empty string
//     {
//         const char* text = "";
//         const char* delimiters = " ";
//         const String str = {.data = (char*)text, .size = 0};
//         const String pattern = {.data = (char*)delimiters, .size = strlen(delimiters)};
//
//         Vector result = string_split(str, pattern);
//
//         // Verify we have 0 substrings
//         assert_int_equal(result.size, 0);
//         assert_null(result.data);
//     }
// }

int main(int argc, char** argv)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_string_buffer_new),
        cmocka_unit_test(test_string_buffer_is_ok),
        cmocka_unit_test(test_string_buffer_new_error),
        cmocka_unit_test(test_string_buffer_read_file),
        cmocka_unit_test(test_string_buffer_read_file_fails),
        cmocka_unit_test(test_string_buffer_to_iterator),
        cmocka_unit_test(test_string_buffer_to_iterator_bad_buffer),
        cmocka_unit_test(test_string_iterator_next_line),
        cmocka_unit_test(test_string_iterator_next_line_all),
        cmocka_unit_test(test_string_iterator_next_line_ignore_multiple_new_lines),
        cmocka_unit_test(test_string_iterator_next_raw_line_new_line),
        cmocka_unit_test(test_string_iterator_next_raw_line_combined),
        cmocka_unit_test(test_string_iterator_next_raw_line_multiple),
        cmocka_unit_test(test_string_trim),
        cmocka_unit_test(test_string_trim_beginning),
        cmocka_unit_test(test_string_trim_end),
        cmocka_unit_test(test_string_at_zero),
        cmocka_unit_test(test_string_at_indexes),
        cmocka_unit_test(test_string_at_invalid),
        cmocka_unit_test(test_split_line_gets_name),
        cmocka_unit_test(test_split_line_gets_arg1),
        cmocka_unit_test(test_split_line_gets_arg2),
        cmocka_unit_test(test_split_line_gets_single_arg),
        cmocka_unit_test(test_split_line_complex_opcode),

    };

    return cmocka_run_group_tests(tests, nullptr, nullptr);
}
