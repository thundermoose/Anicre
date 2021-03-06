#ifndef __INDEXOUTPUT_H__
#define __INDEXOUTPUT_H__
#include <stdint.h>
#include <stdlib.h>
#include "anicr_config.h"
#if CFG_IND_TABLES
extern uint64_t* sp_comb_ind_tables;
extern size_t num_sp_comb_ind_tables;
void initiate_index_file(size_t dim,
			 size_t max_hash_memory_use);

void setup_basis_files();

void setup_basis_file(int energy);

void new_output_block(int energy_in, int energy_out,
		int M_in, int M_out,
		int difference_E,int difference_M,
		int depth);

void close_output_block();

void write_output(uint64_t i, uint64_t j,
		int sgn,
#if CFG_ANICR_ONE
		int ain,
#elif CFG_ANICR_TWO
		int ain,
		int bin,
#elif CFG_ANICR_THREE
		int ain,
		int bin,
		int cin,
#endif
#if CFG_ANICR_ONE
		int aout
#elif CFG_ANICR_TWO
		int aout,
		int bout
#elif CFG_ANICR_THREE
		int aout,
		int bout,
		int cout
#endif
);
//void write_marker(char* str);
void finalize_index_files();
#endif

#endif
