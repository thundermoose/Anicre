#include "indexoutput.h"
#include "anicr_tables.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#if CFG_IND_TABLES
#define CFG_IND_OLD 0
#if CFG_IND_OLD
FILE* outputfile;
#endif
#if !CFG_IND_OLD
FILE* header;
int outputfilePos_num_writes = 0;
char outputfilePos_filename[256];
FILE* outputfilePos = NULL;
int outputfileNeg_num_writes = 0;
char outputfileNeg_filename[256];
FILE* outputfileNeg = NULL;
//uint64_t* sp_in_out_comb;
//size_t num_in_out_comb;
uint64_t* sp_comb_hash_keys;
size_t* sp_comb_hash_indices;
size_t max_num_sp_comb_hash;
char foldername[256];

void set_up(){
  //num_in_out_comb = (num_sp_comb*(num_sp_comb+1))/2;
  //sp_in_out_comb = (uint64_t*)malloc(sizeof(uint64_t)*num_in_out_comb);
  max_num_sp_comb_hash = num_sp_comb_ind_tables*4;
  sp_comb_hash_keys = (uint64_t*)malloc(sizeof(uint64_t)*max_num_sp_comb_hash);
  sp_comb_hash_indices = (size_t*)malloc(sizeof(size_t)*max_num_sp_comb_hash);
  size_t i;
  for (i = 0; i<max_num_sp_comb_hash; i++){
    sp_comb_hash_indices[i] = (uint64_t)-1;
  }
  for (i = 0; i<num_sp_comb_ind_tables; i++){
    uint64_t key = sp_comb_ind_tables[i];
    size_t hash = (key^(key<<16));
    while (sp_comb_hash_indices[hash%max_num_sp_comb_hash] != (uint64_t)-1){
      hash++;
    }
    sp_comb_hash_keys[hash%max_num_sp_comb_hash]=key;
    sp_comb_hash_indices[hash%max_num_sp_comb_hash] = i;
  }

}

size_t getIndex(size_t comb){
  uint64_t key = comb;
  size_t hash = (key^(key<<16));
  while (sp_comb_hash_indices[hash%max_num_sp_comb_hash] != (uint64_t)-1 &&
	 sp_comb_hash_keys[hash%max_num_sp_comb_hash] != key){
    hash++;
  }
  return sp_comb_hash_indices[hash%max_num_sp_comb_hash];
}

#endif
void initFile(size_t dim){
#if CFG_IND_OLD
  char filename[256];
  sprintf(filename,"%s_index_list",CFG_ANICR_IDENT);
  outputfile = fopen(filename,"w");
  fprintf(outputfile,"dim %ld\n",dim);
#endif
#if !CFG_IND_OLD
  
  sprintf(foldername,"%s_index_lists",CFG_ANICR_IDENT);
  { // check if folder exists, and if not create it
    struct stat sb;
    if (stat(foldername,&sb)!=0 || !S_ISDIR(sb.st_mode)){
      mkdir(foldername,0700);
    }
  }
  char filename[256];
  sprintf(filename,"%s/header",foldername);
  header = fopen(filename,"w");
  fprintf(header,"Dim: %ld\n",dim);
#if CFG_ANICR_ONE
  fprintf(header,"#particles: 1\n");
#elif CFG_ANICR_TWO
  fprintf(header,"#particles: 2\n");
#elif CFG_ANICR_THREE
  fprintf(header,"#particles: 3\n");
#endif
  /*
  sprintf(filename,"%s_index_list_pos",CFG_ANICR_IDENT);
  fprintf(header,"Positive indices: %s\n",filename);
  outputfilePos = fopen(filename,"w");
  sprintf(filename,"%s_index_list_neg",CFG_ANICR_IDENT);
  fprintf(header,"Negagive indices: %s\n",filename);
  outputfileNeg = fopen(filename,"w");
  */
  sprintf(filename,"%s/conf_list",foldername);
  fprintf(header,"Configurations: %s\n",filename);
  FILE* confFile = fopen(filename,"w");
  size_t i;
  for (i = 0; i<num_sp_comb_ind_tables; i++){
    uint64_t comb = sp_comb_ind_tables[i];
    uint64_t outcomb;
    #if CFG_ANICR_ONE
    outcomb = (uint64_t)(_table_sp_states[comb&0xFFFF]._spi);
    #elif CFG_ANICR_TWO
    outcomb = (uint64_t)(_table_sp_states[comb&0xFFFF]._spi)|((uint64_t)(_table_sp_states[(comb>>16)&0xFFFF]._spi)<<16);
    #elif CFG_ANICR_THREE
    outcomb = (uint64_t)(_table_sp_states[comb&0xFFFF]._spi)|((uint64_t)(_table_sp_states[(comb>>16)&0xFFFF]._spi)<<16)|((uint64_t)(_table_sp_states[(comb>>32)&0xFFFF]._spi)<<32);
    #endif
    fwrite(&outcomb,sizeof(uint64_t),1,confFile);
  }
  fclose(confFile);
  set_up();
#endif
  fprintf(header,"IndexLists:\n");
}

void newOutputBlock(int E_in, int E_out,
		    int M_in, int M_out,
		    int diff_E,int diff_M,
		    int depth){
  char filename[256];
  if (outputfilePos != NULL){
    fclose(outputfilePos);
    outputfilePos = NULL;
    //if (outputfilePos_num_writes==0){
      //remove(outputfilePos_filename);
      //usleep(10000);
      //printf("removed file: %s\n",outputfilePos_filename);
      //}else{
    fprintf(header,"%s\n",outputfilePos_filename);
      //printf("did not remove file: %s\n",outputfilePos_filename);
      //}
  }
  outputfilePos_num_writes = 0;
  if (outputfileNeg != NULL){
    fclose(outputfileNeg);
    outputfileNeg = NULL;
    //if (outputfileNeg_num_writes==0){
    //remove(outputfileNeg_filename);
      //usleep(10000);
      //printf("removed file: %s\n",outputfileNeg_filename);
    //}else{
    fprintf(header,"%s\n",outputfileNeg_filename);
      //printf("did not remove file: %s\n",outputfileNeg_filename);
      //}
  }
  outputfileNeg_num_writes = 0;
  sprintf(outputfilePos_filename,
	  "%s/index_list_E_in%d_E_out%d_M_in%d_M_out%d_dE%d_dM%d_depth%d_pos",
	  foldername,E_in,E_out,M_in,M_out,diff_E,diff_M,depth);
  outputfilePos = fopen(outputfilePos_filename,"w");
  if (outputfilePos == NULL){
    fprintf(stderr,"file_name: %s\n",outputfilePos_filename);
    fprintf(stderr,"Something is terrible wrong, %s\n",
	    strerror(errno));
    exit(1);
  }
  sprintf(outputfileNeg_filename,
	  "%s/index_list_E_in%d_E_out%d_M_in%d_M_out%d_dE%d_dM%d_depth%d_neg",
	  foldername,E_in,E_out,M_in,M_out,diff_E,diff_M,depth);
  outputfileNeg = fopen(outputfileNeg_filename,"w");
  if (outputfileNeg == NULL){
    fprintf(stderr,"file_name: %s\n",outputfileNeg_filename);
    fprintf(stderr,"Something is terrible wrong, %s\n",
	    strerror(errno));
    exit(1);
  }
}

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
		 ){
  //if (i>j)
  //return;
#if !CFG_IND_OLD
  uint64_t combin = (uint64_t)ain&0xFFFF;
  uint64_t combout = (uint64_t)aout&0xFFFF;
#if CFG_ANICR_TWO || CFG_ANICR_THREE
  combin |= ((uint64_t)bin&0xFFFF)<<16;
  combout|= ((uint64_t)bout&0xFFFF)<<16;
#endif
#if CFG_ANICR_THREE
  combin |= ((uint64_t)cin&0xFFFF)<<32;
  combout|= ((uint64_t)cout&0xFFFF)<<32;
#endif
  size_t i_in = getIndex(combin);
  if (i_in == (uint64_t)-1){
    fprintf(stderr,"Couldn't find combination (combin) in hash table\n");
    fprintf(stderr,"combin = %lx\n",combin);
    fprintf(stderr,"%ld %ld\n",i,j);
    exit(1);
  }
  size_t i_out = getIndex(combout);
  if (i_out == (uint64_t)-1){
    fprintf(stderr,"Couldn't find combination (combout) in hash table\n");
    fprintf(stderr,"combout = %lx\n",combout);
    exit(1);
  }
  // To square matrix
  size_t k = i_in*num_sp_comb_ind_tables+i_out;

  
  if (sgn>0){
    outputfilePos_num_writes++;
    //printf("This happen\n");
  }else{
    outputfileNeg_num_writes++;
  }
  FILE* outputfile = sgn>0 ? outputfilePos : outputfileNeg;
  fprintf(outputfile,"%ld %ld %ld\n",
	  i,j,k);
#endif
#if CFG_IND_OLD
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
#endif
}

void writeMarker(char* str){
  fprintf(outputfilePos,"%s\n",str);
  fprintf(outputfileNeg,"%s\n",str);
}

void closeFile(){
#if !CFG_IND_OLD
  fclose(outputfilePos);
  fclose(outputfileNeg);
  fclose(header);
#endif
#if CFG_IND_OLD
  fclose(outputfile);
#endif
}


#endif
