#ifndef GB_Z80_ASM_OPCODE_TESTS_H
#define GB_Z80_ASM_OPCODE_TESTS_H

void test_split_line_gets_name(void** state);
void test_split_line_gets_arg1(void** state);
void test_split_line_gets_arg2(void** state);
void test_split_line_gets_single_arg(void** state);
void test_split_line_complex_opcode(void** state);
void test_split_line_spaces_between_operands(void** state);
void test_split_line_invalid_parameters(void** state);
void test_split_line_bad_commas(void** state);
void test_parse_opcode_simple(void** state);
void test_parse_opcode_no_operands(void** state);
void test_parse_opcode_single_operand(void** state);
void test_parse_opcode_two_operands(void** state);
void test_parse_opcode_single_variable_operand(void** state);
void test_parse_opcode_second_param_variable(void** state);
void test_search_opcode(void** state);

#endif //GB_Z80_ASM_OPCODE_TESTS_H