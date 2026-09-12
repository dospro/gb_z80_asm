#ifndef GB_Z80_ASM_SYMBOL_TESTS_H
#define GB_Z80_ASM_SYMBOL_TESTS_H

void test_symbol_table_new_creates_empty_table(void** state);
void test_symbol_table_new_rejects_overflowing_capacity(void** state);
void test_symbol_table_new_with_allocator_uses_injected_allocator(void** state);
void test_symbol_table_add_symbol_default_case(void** state);
void test_symbol_table_add_symbol_reallocates(void** state);

#endif //GB_Z80_ASM_SYMBOL_TESTS_H
