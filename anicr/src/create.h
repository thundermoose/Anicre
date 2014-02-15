
#include <stdint.h>

void ammend_tables();

void annihilate_states(int *in_sp_other,
		       int *in_sp);

void annihilate_packed_states(uint64_t *packed);

int find_mp_state(uint64_t *lookfor);

void alloc_accumulate();

void couple_accumulate();
