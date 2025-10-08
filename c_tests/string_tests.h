#ifndef GB_Z80_ASM_STRING_TEST_H
#define GB_Z80_ASM_STRING_TEST_H

void test_string_trim(void** state);
void test_string_trim_beginning(void** state);
void test_string_trim_end(void** state);
void test_string_at_zero(void** state);
void test_string_at_indexes(void** state);
void test_string_at_invalid(void** state);
void test_string_contains_char_false(void** state);
void test_string_contains_char_true(void** state);
void test_string_from_string_buffer(void** state);

#endif //GB_Z80_ASM_STRING_TEST_H