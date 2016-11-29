#ifndef __INDEXOUTPUT_H__
#define __INDEXOUTPUT_H__
#include <stdint.h>
#include <stdlib.h>
#include "anicr_config.h"
#if CFG_IND_TABLES
uint64_t* sp_comb_ind_tables;
size_t num_sp_comb_ind_tables;
void initFile(size_t dim);
void writeOutput(uint64_t i, uint64_t j,
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
void closeFile();
#endif
		
#endif
