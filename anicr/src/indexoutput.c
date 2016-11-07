#include "indexoutput.h"
#include <stdio.h>
#if CFG_IND_TABLES
FILE* outputfile;

void initFile(size_t dim){
  char filename[256];
  sprintf(filename,"%s_index_list",CFG_ANICR_IDENT);
  outputfile = fopen(filename,"w");
  fprintf(outputfile,"dim %ld\n",dim);
}

void writeOutput(uint64_t i, uint64_t j,
		 int sgn,
		 int ain,
#if !CFG_ANICR_ONE
		 int bin,
#elif CFG_ANICR_THREE
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
		 ){
#if CFG_ANICR_ONE
  fprintf(outputfile,"%ld %ld %d %d %d\n",
	  i,j,sgn,aout,ain);
#elif CFG_ANICR_TWO
  fprintf(outputfile,"%ld %ld %d %d %d %d %d\n",
	  i,j,sgn,aout,bout,ain,bin);
#elif CFG_ANICR_THREE
  fprintf(outputfile,"%ld %ld %d %d %d %d %d %d %d\n",
	  i,j,sgn,aout,bout,cout,ain,bin,cin);
#endif

}

void closeFile(){
  fclose(outputfile);
}


#endif
