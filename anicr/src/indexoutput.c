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
int outputfile_positive_num_writes = 0;
char outputfile_positive_filename[256];
FILE* outputfile_positive = NULL;
int outputfile_negative_num_writes = 0;
char outputfile_negative_filename[256];
FILE* outputfile_negative = NULL;
//uint64_t* sp_in_out_comb;
//size_t num_in_out_comb;
uint64_t* sp_comb_hash_keys;
size_t* sp_comb_hash_indices;
size_t max_num_sp_comb_hash;
char foldername[256];

void set_up()
{
	//num_in_out_comb = (num_sp_comb*(num_sp_comb+1))/2;
	//sp_in_out_comb = (uint64_t*)malloc(sizeof(uint64_t)*num_in_out_comb);
	max_num_sp_comb_hash = num_sp_comb_ind_tables*4;
	sp_comb_hash_keys = (uint64_t*)malloc(sizeof(uint64_t)*max_num_sp_comb_hash);
	sp_comb_hash_indices = (size_t*)malloc(sizeof(size_t)*max_num_sp_comb_hash);
	size_t i;
	for (i = 0; i<max_num_sp_comb_hash; i++)
	{
		sp_comb_hash_indices[i] = (uint64_t)-1;
	}
	for (i = 0; i<num_sp_comb_ind_tables; i++)
	{
		uint64_t key = sp_comb_ind_tables[i];
		size_t hash = (key^(key<<16));
		while (sp_comb_hash_indices[hash%max_num_sp_comb_hash] != (uint64_t)-1)
		{
			hash++;
		}
		sp_comb_hash_keys[hash%max_num_sp_comb_hash]=key;
		sp_comb_hash_indices[hash%max_num_sp_comb_hash] = i;
	}

}

size_t get_index(size_t comb)
{
	uint64_t key = comb;
	size_t hash = (key^(key<<16));
	while (sp_comb_hash_indices[hash%max_num_sp_comb_hash] != (uint64_t)-1 &&
			sp_comb_hash_keys[hash%max_num_sp_comb_hash] != key)
	{
		hash++;
	}
	return sp_comb_hash_indices[hash%max_num_sp_comb_hash];
}

#endif
void initFile(size_t dim)
{
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
		if (stat(foldername,&sb)!=0 || !S_ISDIR(sb.st_mode))
		{
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
	sprintf(filename,"%s/conf_list",foldername);
	fprintf(header,"Configurations: %s\n",filename);
	FILE* confFile = fopen(filename,"w");
	size_t i;
	for (i = 0; i<num_sp_comb_ind_tables; i++)
	{
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
		int depth)
{
	char filename[256];
	if (outputfile_positive != NULL)
	{
		fclose(outputfile_positive);
		outputfile_positive = NULL;
		//if (outputfile_positive_num_writes==0)
		{
			//remove(outputfile_positive_filename);
			//usleep(10000);
			//printf("removed file: %s\n",outputfile_positive_filename);
			//}else{
		fprintf(header,"%s\n",outputfile_positive_filename);
		//printf("did not remove file: %s\n",outputfile_positive_filename);
		//}
	}
	outputfile_positive_num_writes = 0;
	if (outputfile_negative != NULL)
	{
		fclose(outputfile_negative);
		outputfile_negative = NULL;
		//if (outputfile_negative_num_writes==0)
		{
			//remove(outputfile_negative_filename);
			//usleep(10000);
			//printf("removed file: %s\n",outputfile_negative_filename);
			//}else{
		fprintf(header,"%s\n",outputfile_negative_filename);
		//printf("did not remove file: %s\n",outputfile_negative_filename);
		//}
	}
	outputfile_negative_num_writes = 0;
	sprintf(outputfile_positive_filename,
			"%s/index_list_E_in%d_E_out%d_M_in%d_M_out%d_dE%d_dM%d_depth%d_pos",
			foldername,E_in,E_out,M_in,M_out,diff_E,diff_M,depth);
	outputfile_positive = fopen(outputfilePos_filename,"w");
	if (outputfile_positive == NULL)
	{
		fprintf(stderr,"file_name: %s\n",outputfile_positive_filename);
		fprintf(stderr,"Something is terrible wrong, %s\n",
				strerror(errno));
		exit(1);
	}
	sprintf(outputfile_negative_filename,
			"%s/index_list_E_in%d_E_out%d_M_in%d_M_out%d_dE%d_dM%d_depth%d_neg",
			foldername,E_in,E_out,M_in,M_out,diff_E,diff_M,depth);
	outputfile_negative = fopen(outputfileNeg_filename,"w");
	if (outputfile_negative == NULL)
	{
		fprintf(stderr,"file_name: %s\n",outputfile_negative_filename);
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
)
{
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
	size_t i_in = get_index(combin);
	if (i_in == (uint64_t)-1)
	{
		fprintf(stderr,"Couldn't find combination (combin) in hash table\n");
		fprintf(stderr,"combin = %lx\n",combin);
		fprintf(stderr,"%ld %ld\n",i,j);
		exit(1);
	}
	size_t i_out = get_index(combout);
	if (i_out == (uint64_t)-1)
	{
		fprintf(stderr,"Couldn't find combination (combout) in hash table\n");
		fprintf(stderr,"combout = %lx\n",combout);
		exit(1);
	}
	// To square matrix
	size_t k = i_in*num_sp_comb_ind_tables+i_out;


	if (sgn>0)
	{
		outputfile_positive_num_writes++;
		//printf("This happen\n");
	}else{
		outputfile_negative_num_writes++;
	}
	FILE* outputfile = sgn>0 ? outputfile_positive : outputfile_negative;
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

void writeMarker(char* str)
{
	fprintf(outputfile_positive,"%s\n",str);
	fprintf(outputfile_negative,"%s\n",str);
}

void closeFile()
{
#if !CFG_IND_OLD
	fclose(outputfile_positive);
	fclose(outputfile_negative);
	fclose(header);
#else
	fclose(outputfile);
#endif
}


#endif
