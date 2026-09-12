
#ifndef GB_Z80_ASM_TOKENIZER_TESTS_H
#define GB_Z80_ASM_TOKENIZER_TESTS_H

void test_tokenizer_opcode(void** state);
void test_tokenizer_skips_whitespace(void** state);
void test_tokenizer_consecutive_commas(void** state);
void test_tokenizer_empty_line(void** state);
void test_tokenizer_blank_line(void** state);
void test_tokenizer_end_is_sticky(void** state);
void test_tokenizer_string_literal(void** state);
void test_tokenizer_empty_string_literal(void** state);
void test_tokenizer_unterminated_string(void** state);

#endif //GB_Z80_ASM_TOKENIZER_TESTS_H