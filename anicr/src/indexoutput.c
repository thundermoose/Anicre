#include "indexoutput.h"
#include "anicr_tables.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#if CFG_IND_TABLES
const size_t no_index = (size_t)-1;
FILE* header = NULL;
int outputfile_positive_num_writes = 0;
char outputfile_positive_filename[256];
FILE* outputfile_positive = NULL;
int outputfile_negative_num_writes = 0;
char outputfile_negative_filename[256];
FILE* outputfile_negative = NULL;
FILE* non_empty_block_record = NULL;
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

typedef struct _sp_comb_hash_
{
	int difference_energy;
	int difference_M;
	int depth;
	configuration_t* keys;
	size_t* indices;
	size_t num_buckets;
} sp_comb_hash_t;

sp_comb_hash_t *current_sp_comb_hash = NULL;

typedef struct
{
	sp_comb_hash_t **buckets;
	size_t num_buckets;
} block_hash_t;

block_hash_t block_hash = {NULL};

char foldername[256];

#define PARTICLE1 (comb&0x000000000000FFFF)
#define PARTICLE2 ((comb&0x00000000FFFF0000)>>16)
#define PARTICLE3 ((comb&0x0000FFFF00000000)>>32)

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

size_t find_block_start(int energy)
{
	for (size_t i = 0; i<num_sp_comb_ind_tables; i++)
	{
		uint64_t comb = sp_comb_ind_tables[i];
		printf("comb.E = %d\n",combination_energy(comb));
		if (energy == combination_energy(comb))
			return i;
	}
	return no_index;
}

size_t determine_block_length(size_t block_start,
			      int energy)
{
	if (block_start == no_index)
		return 0;
	for (size_t i = block_start; i<num_sp_comb_ind_tables; i++)
	{
		uint64_t comb = sp_comb_ind_tables[i];
		if (energy != combination_energy(comb))
			return i-block_start+1;
	}
	return num_sp_comb_ind_tables-block_start;
}

uint64_t configuration_hash_value(configuration_t configuration)
{
	uint64_t h = (uint64_t)(configuration.a_in) ^
	       	((uint64_t)(configuration.a_out)<<47);
#if !CFG_ANICR_ONE
	h ^= ((uint64_t)(configuration.b_in)<<13) ^
		((uint64_t)(configuration.b_out)<<31);
#if !CFG_ANICR_TWO
	h ^= ((uint64_t)(configuration.b_in)<<37) ^
		((uint64_t)(configuration.b_out)<<17);
#endif
#endif
	const uint64_t magic_number = 0xFEDCBA9876543210;
	return h^magic_number;
}

int compare_configurations(configuration_t configuration_a,
			   configuration_t configuration_b)
{
#if CFG_ANICR_ONE
	int diff = configuration_a.a_in - configuration_b.a_in; 	
	if (diff)
		return diff;
	return configuration_a.a_out - configuration_b.a_out;
#elif CFG_ANICR_TWO
	int diff = configuration_a.a_in - configuration_b.a_in; 	
	if (diff)
		return diff;
	diff = configuration_a.b_in - configuration_b.b_in; 	
	if (diff)
		return diff;
	diff = configuration_a.b_out - configuration_b.b_out; 	
	if (diff)
		return diff;
	return configuration_a.a_out - configuration_b.a_out;
#elif CFG_ANICR_THREE
	int diff = configuration_a.a_in - configuration_b.a_in; 	
	if (diff)
		return diff;
	diff = configuration_a.b_in - configuration_b.b_in; 	
	if (diff)
		return diff;
	diff = configuration_a.c_in - configuration_b.c_in; 	
	if (diff)
		return diff;
	diff = configuration_a.c_out - configuration_b.c_out; 	
	if (diff)
		return diff;
	diff = configuration_a.b_out - configuration_b.b_out; 	
	if (diff)
		return diff;
	return configuration_a.a_out - configuration_b.a_out;
#endif
}

size_t find_bucket(const sp_comb_hash_t *sp_comb_hash,
		   configuration_t configuration)
{
	uint64_t hash = configuration_hash_value(configuration);
	size_t bucket_index = hash % sp_comb_hash->num_buckets;
	while (sp_comb_hash->indices[bucket_index] != no_index &&
	       compare_configurations(sp_comb_hash->keys[bucket_index],
				      configuration) != 0)
	{
		hash++;
		bucket_index = hash % sp_comb_hash->num_buckets;
	}
	return bucket_index;
}

void insert_configuration(sp_comb_hash_t *sp_comb_hash,
			  configuration_t configuration,
			  size_t index)
{
	size_t bucket_index = find_bucket(sp_comb_hash, configuration);
	sp_comb_hash->keys[bucket_index] = configuration;
	sp_comb_hash->indices[bucket_index] = index;
}

size_t get_configuration_index(sp_comb_hash_t *sp_comb_hash,
			       configuration_t configuration)
{
	return sp_comb_hash->indices[find_bucket(sp_comb_hash,configuration)];	
}

sp_comb_hash_t *setup_sp_comb_basis_and_hash(int difference_energy,
					     int difference_M,
					     int depth_in)
{
       	int depth_out = difference_energy+depth_in;	
	size_t in_block_start = find_block_start(depth_in);
	size_t out_block_start = find_block_start(depth_out);
	size_t in_block_length = determine_block_length(in_block_start,
						       	depth_in);
	size_t out_block_length = determine_block_length(out_block_start,
							 depth_out);
	size_t num_configurations = in_block_length*out_block_length;
	if (num_configurations == 0)
		return NULL;
	configuration_t *configurations =
	       	(configuration_t*)
		malloc(num_configurations*sizeof(configuration_t));
	sp_comb_hash_t *sp_comb_hash =
	       	(sp_comb_hash_t*)malloc(sizeof(sp_comb_hash_t));
	sp_comb_hash->difference_energy = difference_energy;
	sp_comb_hash->difference_M = difference_M;
	sp_comb_hash->depth = depth_in;
	sp_comb_hash->num_buckets = num_configurations;
	sp_comb_hash->keys =
	       	(configuration_t*)calloc(sp_comb_hash->num_buckets,
					 sizeof(configuration_t));
	sp_comb_hash->indices =
	       	(size_t*)malloc(sp_comb_hash->num_buckets*
				sizeof(size_t));
	for (size_t i = 0; i<sp_comb_hash->num_buckets; i++)
		sp_comb_hash->indices[i] = no_index;
	size_t block_index = 0;
	for (size_t i = 0; i<in_block_length; i++)
	{
		uint64_t in_configuration =
		       	sp_comb_ind_tables[i+in_block_start];
		int M_ket = combination_M(in_configuration);
		for (size_t j = 0; j<out_block_length; j++)
		{
			uint64_t out_configuration =
				sp_comb_ind_tables[j+out_block_start];
			int M_bra = combination_M(out_configuration);
			if (M_bra != M_ket+difference_M)
				continue;
			size_t index = i*out_block_length+j;
			configuration_t current = 
			{
#if CFG_ANICR_ONE
				.a_in = (short)(in_configuration & 0x000000000000FFFF),
				.a_out = (short)(out_configuration & 0x000000000000FFFF),
#elif CFG_ANICR_TWO
				.a_in = (short)(in_configuration & 0x000000000000FFFF),
				.b_in = (short)((in_configuration & 0x00000000FFFF0000)>>16),
				.a_out = (short)(out_configuration & 0x000000000000FFFF),
				.b_out = (short)((out_configuration & 0x00000000FFFF0000)>>16),
#elif CFG_ANICR_THREE
				.a_in = (short)(in_configuration & 0x000000000000FFFF),
				.b_in = (short)((in_configuration & 0x00000000FFFF0000)>>16),
				.c_in = (short)((in_configuration & 0x0000FFFF00000000)>>32),
				.a_out = (short)(out_configuration & 0x000000000000FFFF),
				.b_out = (short)((out_configuration & 0x00000000FFFF0000)>>16),
				.c_out = (short)((out_configuration & 0x0000FFFF00000000)>>32),
#endif
			};		
			insert_configuration(sp_comb_hash,current,block_index);	
#if CFG_ANICR_ONE
			current.a_in = _table_sp_states[current.a_in]._spi;
			current.a_out = _table_sp_states[current.a_out]._spi;
#elif CFG_ANICR_TWO
			current.a_in = _table_sp_states[current.a_in]._spi;
			current.b_in = _table_sp_states[current.b_in]._spi;
			current.a_out = _table_sp_states[current.a_out]._spi;
			current.b_out = _table_sp_states[current.b_out]._spi;
#elif CFG_ANICR_THREE
			current.a_in = _table_sp_states[current.a_in]._spi;
			current.b_in = _table_sp_states[current.b_in]._spi;
			current.c_in = _table_sp_states[current.c_in]._spi;
			current.a_out = _table_sp_states[current.a_out]._spi;
			current.b_out = _table_sp_states[current.b_out]._spi;
			current.c_out = _table_sp_states[current.c_out]._spi;
#endif
			configurations[block_index] = current;
			block_index++;
		}
	}
	char file_name[2048];
	sprintf(file_name,"%s/conn_dE_%d_dM_%d_depth_%d",
		foldername,difference_energy,difference_M,depth_in);
	FILE *configuration_file = fopen(file_name,"w");
	if (configuration_file == NULL)
	{
		fprintf(stderr,"Could not open file %s.%s\n",
			file_name,strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (fwrite(configurations,
		   sizeof(configuration_t),
		   block_index,
		   configuration_file)<block_index)
	{
		fprintf(stderr,"Could not write configurations to %s\n",
			file_name);
		exit(EXIT_FAILURE);
	}
	fclose(configuration_file);
	free(configurations);
	return sp_comb_hash;
}

void initiate_index_file(size_t dim,
			 size_t max_num_blocks)
{

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
		outcomb = (uint64_t)(_table_sp_states[comb&0xFFFF]._spi)|
			((uint64_t)(_table_sp_states[(comb>>16)&0xFFFF]._spi)<<16);
#elif CFG_ANICR_THREE
		outcomb = (uint64_t)(_table_sp_states[comb&0xFFFF]._spi)|
			((uint64_t)(_table_sp_states[(comb>>16)&0xFFFF]._spi)<<16)|
			((uint64_t)(_table_sp_states[(comb>>32)&0xFFFF]._spi)<<32);
#endif
		fwrite(&outcomb,sizeof(uint64_t),1,confFile);
	}
	fclose(confFile);
	fprintf(header,"IndexLists:\n");
	block_hash.num_buckets = 4*max_num_blocks;
	block_hash.buckets = (sp_comb_hash_t**)calloc(block_hash.num_buckets,
						      sizeof(sp_comb_hash_t*));
}

uint64_t compute_block_hash(int difference_energy,
			    int difference_M,
			    int depth)
{
	uint64_t h1 = (uint64_t)(difference_energy) |
		((uint64_t)(difference_M)<<31);
	uint64_t h2 = (uint64_t)(depth) |
		((uint64_t)(difference_energy)<<37);
	uint64_t h3 = (uint64_t)(difference_M) |
		((uint64_t)(depth)<<29);
	const uint64_t magic_number = 0xFEDCBA9876543210;
	return h1 ^ h2 ^ h3 ^ magic_number;
}

size_t find_block(int difference_energy,
		  int difference_M,
		  int depth)
{
	uint64_t hash = compute_block_hash(difference_energy,
					   difference_M,
					   depth);
	size_t block_index = hash % block_hash.num_buckets;
	while (block_hash.buckets[block_index] != NULL &&
	       (block_hash.buckets[block_index]->difference_energy !=
	       difference_energy ||
	       block_hash.buckets[block_index]->difference_M != difference_M ||
	       block_hash.buckets[block_index]->depth != depth))
	{
		hash++;
		block_index = hash % block_hash.num_buckets;
	}
	return block_index;
}

void update_current_sp_comb_hash(int difference_energy,
				 int difference_M,
				 int depth)
{
	if (current_sp_comb_hash != NULL &&
	    current_sp_comb_hash->difference_energy == difference_energy &&
	    current_sp_comb_hash->difference_M == difference_M &&
	    current_sp_comb_hash->depth == depth)
		return;
	size_t bucket_index = find_block(difference_energy,
					 difference_M,
					 depth);
	if (block_hash.buckets[bucket_index] == NULL)
		block_hash.buckets[bucket_index] =
		       	setup_sp_comb_basis_and_hash(difference_energy,
						     difference_M,
						     depth);
	current_sp_comb_hash = block_hash.buckets[bucket_index];
}

void new_output_block(int energy_in, int energy_out,
		    int M_in, int M_out,
		    int difference_energy,int difference_M,
		    int depth)
{
	update_current_sp_comb_hash(difference_energy,
				    difference_M,
				    depth);
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
		foldername,
		energy_in,
		energy_out,
		M_in,
		M_out,
		difference_energy,
		difference_M,
		depth);
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
		foldername,
		energy_in,
		energy_out,
		M_in,
		M_out,
		difference_energy,
		difference_M,
		depth);
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
	configuration_t current_configuration =
	{
#if CFG_ANICR_ONE
		.a_in = ain,
		.a_out = aout
#elif CFG_ANICR_TWO
		.a_in = ain,
		.b_in = bin,
		.a_out = aout,
		.b_out = bout
#elif CFG_ANICR_THREE
		.a_in = ain,
		.b_in = bin,
		.c_in = cin,
		.a_out = aout,
		.b_out = bout,
		.c_out = cout
#endif
	};
	size_t k = get_configuration_index(current_sp_comb_hash,
					   current_configuration);
	if (sgn>0)
	{
		outputfile_positive_num_writes++;
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
	fprintf(outputfile,"%ld %ld %ld, %d %d %d %d %d %d\n",
		i,j,k,ain,bin,cin,aout,bout,cout);
#endif
}

void write_marker(char* str)
{
	fprintf(outputfile_positive,"%s\n",str);
	fprintf(outputfile_negative,"%s\n",str);
}

void close_file()
{
	fclose(outputfile_positive);
	fclose(outputfile_negative);
	fclose(header);
}

void free_sp_comb_hash(sp_comb_hash_t *sp_comb_hash)
{
	free(sp_comb_hash->keys);
	free(sp_comb_hash->indices);
	free(sp_comb_hash);
}

void free_block_table()
{
	for (size_t i = 0; i<block_hash.num_buckets; i++)
		if (block_hash.buckets[i] != NULL)
			free_sp_comb_hash(block_hash.buckets[i]);
	free(block_hash.buckets);
}

void finalilze_index_files()
{
	close_file();
	free_block_table();
}

#endif
