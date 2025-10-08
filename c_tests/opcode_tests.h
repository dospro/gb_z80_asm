#ifndef GB_Z80_ASM_OPCODE_TESTS_H
#define GB_Z80_ASM_OPCODE_TESTS_H

void test_split_line_gets_name(void** state);
void test_split_line_gets_arg1(void** state);
void test_split_line_gets_arg2(void** state);
void test_split_line_gets_single_arg(void** state);
void test_split_line_complex_opcode(void** state);
void test_parse_opcode_simple(void** state);
void test_parse_opcode_no_params(void** state);
void test_parse_opcode_single_param(void** state);
void test_parse_opcode_single_variable(void** state);
void test_search_opcode(void** state);

#endif //GB_Z80_ASM_OPCODE_TESTS_H