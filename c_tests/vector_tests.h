#ifndef GB_Z80_ASM_VECTOR_TESTS_H
#define GB_Z80_ASM_VECTOR_TESTS_H

void test_vector_new_creates_empty_vector(void** state);
void test_vector_new_rejects_overflowing_capacity(void** state);
void test_vector_new_rejects_zero_element_size(void** state);
void test_vector_new_with_allocator_uses_injected_allocator(void** state);
void test_vector_new_with_allocator_reports_no_capacity_on_failure(void** state);
void test_vector_free_is_idempotent_and_leaves_vector_reusable(void** state);
void test_vector_push_default_case(void** state);
void test_vector_push_reallocates(void** state);
void test_vector_push_growth_releases_old_storage(void** state);
void test_vector_push_growth_failure_leaves_vector_intact(void** state);
void test_vector_push_grows_from_empty(void** state);
void test_vector_push_rejects_null_arguments(void** state);
void test_vector_at_rejects_out_of_range_index(void** state);

#endif //GB_Z80_ASM_VECTOR_TESTS_H
