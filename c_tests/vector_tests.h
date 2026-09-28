#ifndef GB_Z80_ASM_VECTOR_TESTS_H
#define GB_Z80_ASM_VECTOR_TESTS_H

void test_vector_new_reserves_storage(void** state);
void test_vector_new_uses_injected_allocator(void** state);
void test_vector_free_releases_storage(void** state);
void test_vector_free_is_idempotent(void** state);
void test_vector_free_accepts_null(void** state);
void test_vector_new_zero_capacity_skips_allocation(void** state);
void test_vector_new_allocation_failure(void** state);
void test_vector_new_rejects_zero_element_size(void** state);
void test_vector_new_rejects_capacity_overflow(void** state);
void test_vector_push_stores_one_element(void** state);
void test_vector_push_places_second_element(void** state);
void test_vector_push_copies_source(void** state);
void test_vector_at_returns_first(void** state);
void test_vector_at_returns_second(void** state);
void test_vector_at_rejects_size(void** state);
void test_vector_at_rejects_capacity(void** state);
void test_vector_at_rejects_maximum_index(void** state);
void test_vector_at_rejects_empty(void** state);
void test_vector_at_rejects_null(void** state);
void test_vector_push_rejects_null_vector(void** state);
void test_vector_push_rejects_null_source(void** state);
void test_vector_push_rejects_zero_element_size(void** state);
void test_vector_push_grows_from_empty(void** state);
void test_vector_push_growth_preserves_contents(void** state);
void test_vector_push_growth_uses_allocator(void** state);
void test_vector_push_growth_failure_preserves_state(void** state);
void test_vector_free_leaves_vector_reusable(void** state);
void test_vector_push_recovers_after_failed_reservation(void** state);
void test_vector_append_stores_range(void** state);
void test_vector_append_places_second_range(void** state);
void test_vector_append_rejects_null_vector(void** state);
void test_vector_append_rejects_null_source(void** state);
void test_vector_append_rejects_zero_element_size(void** state);
void test_vector_append_grows_from_empty(void** state);
void test_vector_append_growth_preserves_contents(void** state);
void test_vector_append_growth_uses_allocator(void** state);
void test_vector_append_growth_failure_preserves_state(void** state);
void test_vector_append_bulk_growth_exceeds_doubling(void** state);
void test_vector_append_bulk_growth_exceeds_initial_floor(void** state);
void test_vector_append_zero_count_skips_allocation(void** state);
void test_vector_append_zero_count_preserves_contents(void** state);
void test_vector_append_rejects_count_overflow(void** state);
void test_vector_append_rejects_byte_overflow(void** state);
void test_vector_append_own_subrange_without_growth(void** state);
void test_vector_append_own_subrange_with_growth(void** state);
void test_vector_append_failure_does_not_fill_spare_capacity(void** state);
void test_vector_append_bulk_growth_preserves_prefix(void** state);

#endif //GB_Z80_ASM_VECTOR_TESTS_H
