#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "string_tests.h"
#include "string_buffer_tests.h"
#include "string_iterator_tests.h"
#include "opcode_tests.h"
#include "symbol_tests.h"
#include "tokenizer_tests.h"

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
        cmocka_unit_test(test_string_buffer_append_c_string),
        cmocka_unit_test(test_string_buffer_append_c_string_no_capacity),
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
        cmocka_unit_test(test_split_line_spaces_between_operands),
        cmocka_unit_test(test_split_line_invalid_parameters),
        cmocka_unit_test(test_split_line_bad_commas),
        cmocka_unit_test(test_string_contains_char_false),
        cmocka_unit_test(test_string_contains_char_true),
        cmocka_unit_test(test_string_from_string_buffer),
        cmocka_unit_test(test_parse_opcode_simple),
        cmocka_unit_test(test_parse_opcode_no_operands),
        cmocka_unit_test(test_parse_opcode_single_operand),
        cmocka_unit_test(test_parse_opcode_single_variable_operand),
        cmocka_unit_test(test_parse_opcode_two_operands),
        cmocka_unit_test(test_parse_opcode_second_param_variable),
        cmocka_unit_test(test_search_opcode),
        cmocka_unit_test(test_symbol_table_new_creates_empty_table),
        cmocka_unit_test(test_symbol_table_new_rejects_overflowing_capacity),
        cmocka_unit_test(test_symbol_table_new_with_allocator_uses_injected_allocator),
        cmocka_unit_test(test_symbol_table_add_symbol_default_case),
        cmocka_unit_test(test_symbol_table_add_symbol_reallocates),
        cmocka_unit_test(test_tokenizer_opcode),
        cmocka_unit_test(test_tokenizer_skips_whitespace),
        cmocka_unit_test(test_tokenizer_consecutive_commas),
        cmocka_unit_test(test_tokenizer_empty_line),
        cmocka_unit_test(test_tokenizer_blank_line),
        cmocka_unit_test(test_tokenizer_end_is_sticky),
        cmocka_unit_test(test_tokenizer_string_literal),
        cmocka_unit_test(test_tokenizer_empty_string_literal),
        cmocka_unit_test(test_tokenizer_unterminated_string),

    };

    return cmocka_run_group_tests(tests, nullptr, nullptr);
}
