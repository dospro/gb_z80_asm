#ifndef GB_Z80_ASM_FIXUPS_TESTS_H
#define GB_Z80_ASM_FIXUPS_TESTS_H

void test_fixups_table_new_creates_empty_table(void** state);
void test_fixups_table_new_rejects_overflowing_capacity(void** state);
void test_fixups_table_new_with_allocator_uses_injected_allocator(void** state);
void test_fixups_table_add_fixup_default_case(void** state);
void test_fixups_table_add_fixup_reallocates(void** state);
void test_fixups_table_at_rejects_out_of_range_index(void** state);

#endif //GB_Z80_ASM_FIXUPS_TESTS_H
