#ifndef GB_Z80_ASM_STRING_BUFFER_TESTS_H
#define GB_Z80_ASM_STRING_BUFFER_TESTS_H

void test_string_buffer_new(void** state);
void test_string_buffer_is_ok(void** state);
void test_string_buffer_new_error(void** state);
void test_string_buffer_read_file(void** state);
void test_string_buffer_read_file_fails(void** state);
void test_string_buffer_to_iterator(void** state);

void test_string_buffer_to_iterator_bad_buffer(void** state);

#endif //GB_Z80_ASM_STRING_BUFFER_TESTS_H
