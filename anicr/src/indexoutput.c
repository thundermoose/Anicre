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
const size_t no_index = -1;
FILE* header;
int outputfile_positive_num_writes = 0;
char outputfile_positive_filename[256];
FILE* outputfile_positive = NULL;
int outputfile_negative_num_writes = 0;
char outputfile_negative_filename[256];
FILE* outputfile_negative = NULL;
//uint64_t* sp_in_out_comb;
//size_t num_in_out_comb;
typedef struct
{
#if CFG_ANICR_ONE	
	short a_in,a_out;
#elif CFG_ANICR_TWO
	short a_in,b_in,a_out,b_out;
#elif CFG_ANICR_THREE
	short a_in,b_in,c_in,a_out,b_out,c_out;
#endif
} configuration_t;

struct _sp_comb_hash_
{
	configuration_t* keys;
	size_t* indices;
	size_t num_buckets;
} sp_comb_hash;
char foldername[256];

void initate_sp_comb_hash()
{
	//num_in_out_comb = (num_sp_comb*(num_sp_comb+1))/2;
	//sp_in_out_comb = (uint64_t*)malloc(sizeof(uint64_t)*num_in_out_comb);
	sp_comb_hash.num_buckets = num_sp_comb_ind_tables*4;
	sp_comb_hash.keys = (uint64_t*)malloc(sizeof(uint64_t)*sp_comb_hash.num_buckets);
	sp_comb_hash.indices = (size_t*)malloc(sizeof(size_t)*sp_comb_hash.num_buckets);
	size_t i;
	for (i = 0; i<sp_comb_hash.num_buckets; i++)
	{
		sp_comb_hash.indices[i] = no_index;
	}
	for (i = 0; i<num_sp_comb_ind_tables; i++)
	{
		uint64_t key = sp_comb_ind_tables[i];
		size_t hash = (key^(key<<16));
		while (sp_comb_hash.indices[hash%sp_comb_hash.num_buckets] != (uint64_t)-1)
		{
			hash++;
		}
		sp_comb_hash.keys[hash%sp_comb_hash.num_buckets]=key;
		sp_comb_hash.indices[hash%sp_comb_hash.num_buckets] = i;
	}

}

#define PARTICLE1 comb&0x000000000000FFFF
#define PARTICLE2 comb&0x00000000FFFF0000
#define PARTICLE3 comb&0x0000FFFF00000000

int combination_energy(uint64_t comb)
{
#if CFG_ANICR_ONE
	int comb_energy =
		_table_sp_states[PARTICLE1]._n*2+
		_table_sp_states[PARTICLE1]._l;
#elif CFG_ANICR_TWO
	int comb_energy =
		_table_sp_states[PARTICLE1]._n*2+
		_table_sp_states[PARTICLE1]._l+
		_table_sp_states[PARTICLE2]._n*2+
		_table_sp_states[PARTICLE2]._l;
#elif CFG_ANICR_THREE
	int comb_energy =
		_table_sp_states[PARTICLE1]._n*2+
		_table_sp_states[PARTICLE1]._l+
		_table_sp_states[PARTICLE2]._n*2+
		_table_sp_states[PARTICLE2]._l+
		_table_sp_states[PARTICLE3]._n*2+
		_table_sp_states[PARTICLE3]._l;
#endif
	return comb_energy;
}

int combination_M(uint64_t comb)
{
#if CFG_ANICR_ONE
	int comb_M = 
		_table_sp_states[PARTICLE1]._m;
#elif CFG_ANICR_TWO
	int comb_M = 
		_table_sp_states[PARTICLE1]._m+
		_table_sp_states[PARTICLE2]._m;
#elif CFG_ANICR_THREE
	int comb_M = 
		_table_sp_states[PARTICLE1]._m+
		_table_sp_states[PARTICLE2]._m+
		_table_sp_states[PARTICLE3]._m;
#endif
	return comb_M;
}

size_t find_block_start(int energy,int M)
{
	for (size_t i = 0; i<num_sp_comb_ind_tables; i++)
	{
		uint64_t comb = sp_comb_ind_tables[i];
		if (energy == combination_energy(comb) &&
		    M == combination_M(comb))
			return i;
	}
	return no_index;
}

size_t determine_block_length(size_t block_start,
			      int energy,int M)
{
	for (size_t i = block_start; i<num_sp_comb_ind_tables; i++)
	{
		uint64_t comb = sp_comb_ind_tables[i];
		if (energy != combination_energy(comb) ||
		    M != combination_M(comb))
			return i-block_start+1;
	}
	return num_sp_comb_ind_tables-block_start;
}

void reset_sp_comb_hash(size_t num_configurations)
{
	if (2*num_configurations > sp_comb_hash.num_buckets)
	{
		sp_comb_hash.num_buckets = 2*num_configurations;
		if (sp_comb_hash.keys != NULL)
			free(sp_comb_hash.keys);
		if (sp_comb_hash.indices != NULL)
			free(sp_comb_hash.indices);
		sp_comb_hash.keys =
		       	(configuration_t*)calloc(sp_comb_hash.num_buckets,
						 sizeof(configuration_t));
		sp_comb_hash.indices =
			(size_t*)calloc(sp_comb_hash.num_buckets,
					sizeof(size_t));
	}
	else
	{
		memset(sp_comb_hash.keys,
		       0,
		       sp_comb_hash.num_buckets*sizeof(configuration_t));
		memset(sp_comb_hash.indices,
		       0,
		       sp_comb_hash.num_buckets*sizeof(size_t));
	}
}

void insert_configuration(configuration_t configuration,
			  size_t index)
{
	uint64_t hash = configuration_hash_value(configuration);
	while (sp_comb_hash.
}

void setup_sp_comb_basis_and_hash(int energy_in,
				  int energy_out,
				  int M_in,
				  int M_out,
				  int depth_in)
{
	int difference_energy = energy_in-energy_out;
       	int depth_out = difference_energy+depth_in;	
	size_t in_block_start = find_block_start(depth_in,M_in);
	size_t out_block_start = find_block_start(depth_out,M_out);
	size_t in_block_length = determine_block_length(in_block_start,
							depth_in,M_in);
	size_t out_block_length = determine_block_length(out_block_start,
							 depth_out,M_out);
	size_t num_configurations = in_block_length*out_block_length;
	configuration_t *configurations =
	       	(configuration_t*)malloc(num_configurations*
					 sizeof(configuration_t));
	reset_sp_comb_hash(num_configurations);
	for (size_t i = 0; i<in_block_length; i++)
	{
		uint64_t in_configuration =
		       	sp_comb_ind_tables[i+in_block_start];
		for (size_t j = 0; j<out_block_length; j++)
		{
			uint64_t out_configuration =
				sp_comb_ind_tables[j+out_block_start];
			size_t index = i*out_block_length+j;
			configuration_t current = 
			{
#if CFG_ANICR_ONE
				.a_in = in_configuration & 0x000000000000FFFF,
				.a_out = out_configuration & 0x000000000000FFFF,
#elif CFG_ANICR_TWO
				.a_in = in_configuration & 0x000000000000FFFF,
				.b_in = in_configuration & 0x00000000FFFF0000,
				.a_out = out_configuration & 0x000000000000FFFF,
				.b_out = out_configuration & 0x00000000FFFF0000,
#elif CFG_ANICR_THREE
				.a_in = in_configuration & 0x000000000000FFFF,
				.b_in = in_configuration & 0x00000000FFFF0000,
				.c_in = in_configuration & 0x0000FFFF00000000,
				.a_out = out_configuration & 0x000000000000FFFF,
				.b_out = out_configuration & 0x00000000FFFF0000,
				.c_out = out_configuration & 0x0000FFFF00000000,
#endif
			};		
			configurations[index] = current;
			insert_configuration(current,index);	
		}
	}
	char file_name[2048];
	sprintf(file_name,"%s/conn_E_in_%d_M_in_%d_E_out_%d_M_out_%d_depth_%d",
		foldername,energy_in,M_in,energy_out,M_out,depth_in);
	FILE *configuration_file = fopen(file_name,"w");
	if (configuration_file == NULL)
	{
		fprintf(stderr,"Could not open file %s.%s\n",
			file_name,strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (fwrite(configurations,
		   sizeof(configuration_t),
		   num_configurations,
		   configuration_file)<num_configurations)
	{
		fprintf(stderr,"Could not write configurations to %s\n",
			file_name);
		exit(EXIT_FAILURE);
	}
	fclose(configuration_file);
	free(configurations);
}

size_t get_index(size_t comb)
{
	uint64_t key = comb;
	size_t hash = (key^(key<<16));
	while (sp_comb_hash.indices[hash%sp_comb_hash.num_buckets] != (uint64_t)-1 &&
	       sp_comb_hash.keys[hash%sp_comb_hash.num_buckets] != key)
	{
		hash++;
	}
	return sp_comb_hash.indices[hash%sp_comb_hash.num_buckets];
}

#endif
void initiate_index_file(size_t dim)
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
	initate_sp_comb_hash();
#endif
	fprintf(header,"IndexLists:\n");
}

void new_output_block(int energy_in, int energy_out,
		    int M_in, int M_out,
		    int difference_energy,int difference_M,
		    int depth)
{
	char filename[256];
	if (outputfile_positive != NULL)
	{
		fclose(outputfile_positive);
		outputfile_positive = NULL;
		fprintf(header,"%s\n",outputfile_positive_filename);
	}
	outputfile_positive_num_writes = 0;
	if (outputfile_negative != NULL)
	{
		fclose(outputfile_negative);
		outputfile_negative = NULL;
		fprintf(header,"%s\n",outputfile_negative_filename);
	}
	outputfile_negative_num_writes = 0;
	sprintf(outputfile_positive_filename,
		"%s/index_list_E_in%d_E_out%d_M_in%d_M_out%d_dE%d_dM%d_depth%d_pos",
		foldername,energy_in,energy_out,M_in,M_out,difference_energy,difference_M,depth);
	outputfile_positive = fopen(outputfile_positive_filename,"w");
	if (outputfile_positive == NULL)
	{
		fprintf(stderr,"file_name: %s\n",outputfile_positive_filename);
		fprintf(stderr,"Something is terrible wrong, %s\n",
			strerror(errno));
		exit(1);
	}
	sprintf(outputfile_negative_filename,
		"%s/index_list_E_in%d_E_out%d_M_in%d_M_out%d_dE%d_dM%d_depth%d_neg",
		foldername,energy_in,energy_out,M_in,M_out,difference_energy,difference_M,depth);
	outputfile_negative = fopen(outputfile_negative_filename,"w");
	if (outputfile_negative == NULL)
	{
		fprintf(stderr,"file_name: %s\n",outputfile_negative_filename);
		fprintf(stderr,"Something is terrible wrong, %s\n",
			strerror(errno));
		exit(1);
	}
}

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
#if CFG_ANICR_ONE
	fprintf(outputfile,"%ld %ld %ld, %d %d\n",
		i,j,k,ain,aout);
#elif CFG_ANICR_TWO
	fprintf(outputfile,"%ld %ld %ld, %d %d %d %d\n",
		i,j,k,ain,bin,aout,bout);
#elif CFG_ANICR_THREE
	fprintf(outputfile,"%ld %ld %ld, %d %d %d %d\n",
		i,j,k,ain,bin,aout,bout);
#endif
	//fprintf(outputfile,"%ld %ld %ld\n",
	//	i,j,k);
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

void write_marker(char* str)
{
	fprintf(outputfile_positive,"%s\n",str);
	fprintf(outputfile_negative,"%s\n",str);
}

void close_file()
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
