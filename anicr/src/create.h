
#include <stdint.h>

void ammend_tables();

void annihilate_states(int *in_sp_other,
		       int *in_sp
#if CFG_CONN_TABLES || CFG_IND_TABLES
		       ,
		       int miss_parity, int miss_m, int miss_E,
		       int depth
#endif
		       );

void annihilate_packed_states(uint64_t *packed
#if CFG_CONN_TABLES || CFG_IND_TABLES
			      ,
			      int miss_parity, int miss_m, int miss_E,
			      int depth
#endif
			      );

int find_mp_state(uint64_t *lookfor, double *val);

void alloc_accumulate();

void couple_accumulate();

void couple_accumulate_2();
