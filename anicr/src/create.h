
#include <stdint.h>

void ammend_tables();

void annihilate_states(int *in_sp_other,
		       int *in_sp
#if CFG_CONN_TABLES
		       ,
		       int miss_parity, int miss_m, int miss_E
#endif
		       );

void annihilate_packed_states(uint64_t *packed
#if CFG_CONN_TABLES
			      ,
			      int miss_parity, int miss_m, int miss_E
#endif
			      );

int find_mp_state(uint64_t *lookfor, double *val);

void alloc_accumulate();

void couple_accumulate();

void couple_accumulate_2();
