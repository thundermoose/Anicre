#include "indexoutput.h"
#include "anicr_tables.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#if CFG_IND_TABLES

// Macros
#define PARTICLE1 (comb&first_particle_mask)
#define PARTICLE2 ((comb&second_particle_mask)>>16)
#define PARTICLE3 ((comb&third_particle_mask)>>32)

#define INDEX_TRIPLE_BUFFER_SIZE 4096

// Constants
const size_t no_index = (size_t)-1;

const uint64_t first_particle_mask  = 0x000000000000FFFF;
const uint64_t second_particle_mask = 0x00000000FFFF0000;
const uint64_t third_particle_mask  = 0x0000FFFF00000000;

typedef struct
{
	int i,j,k;
} index_triple_t;

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

typedef struct
{
#if CFG_ANICR_ONE	
	short a;
#elif CFG_ANICR_TWO
	short a,b;
#elif CFG_ANICR_THREE
	short a,b,c;
#endif
} basis_state_t;

typedef struct _sp_comb_hash_
{
	int difference_energy;
	int difference_M;
	int depth;
	configuration_t* keys;
	size_t* indices;
	size_t num_buckets;
} sp_comb_hash_t;


typedef struct
{
	sp_comb_hash_t **buckets;
	size_t num_buckets;
	size_t used_size;
} block_hash_t;

// Evil global variables
FILE* header = NULL;
char outputfile_filename[256];
FILE* outputfile= NULL;
FILE* non_empty_block_record = NULL;
uint64_t* sp_comb_ind_tables;
size_t num_sp_comb_ind_tables;
size_t indin;
size_t max_used;
sp_comb_hash_t *current_sp_comb_hash = NULL;
block_hash_t block_hash = {NULL};

char foldername[256];

index_triple_t index_triple_buffer[INDEX_TRIPLE_BUFFER_SIZE];
size_t num_triples_in_buffer = 0;


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
		//printf("comb.E = %d\n",combination_energy(comb));
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
	for (size_t i = block_start+1; i<num_sp_comb_ind_tables; i++)
	{
		uint64_t comb = sp_comb_ind_tables[i];
		if (energy != combination_energy(comb))
			return i-block_start;
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
	if (bucket_index == no_index)
		return;
	sp_comb_hash->keys[bucket_index] = configuration;
	sp_comb_hash->indices[bucket_index] = index;
}

size_t get_configuration_index(sp_comb_hash_t *sp_comb_hash,
			       configuration_t configuration)
{
	size_t bucket_index = find_bucket(sp_comb_hash,configuration);
	if (bucket_index == no_index)
		return no_index;
	return sp_comb_hash->indices[bucket_index];	
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
	//printf("Configurations in (%d %d %d)\n",
	//       difference_energy,
	//       difference_M,
	//       depth_in);
	int is_diagonal = difference_energy == 0 && difference_M == 0;
	for (size_t i = 0; i<in_block_length; i++)
	{
		uint64_t in_configuration =
		       	sp_comb_ind_tables[i+in_block_start];
		int M_ket = combination_M(in_configuration);
		int E1 = combination_energy(in_configuration);
		if (E1 != depth_in)
			break;
		for (size_t j = 0; j<out_block_length; j++)
		{
			uint64_t out_configuration =
				sp_comb_ind_tables[j+out_block_start];
			int M_bra = combination_M(out_configuration);
			int E2 = combination_energy(out_configuration);
			if (E2 != depth_out)
				break;
			//printf("M_ket = %d, M_bra = %d\n",
			//       M_ket,M_bra);
			
			if (M_bra - M_ket != difference_M)
				continue;
			size_t index = i*out_block_length+j;
			configuration_t current = 
			{
#if CFG_ANICR_ONE
				.a_in  = (short)(in_configuration   & 
						 first_particle_mask),
				.a_out = (short)(out_configuration  & 
						 first_particle_mask),
#elif CFG_ANICR_TWO
				.a_in  = (short)(in_configuration   & 
						 first_particle_mask),
				.a_out = (short)(out_configuration  & 
						 first_particle_mask),
				.b_in  = (short)((in_configuration  & 
						  second_particle_mask)>>16),
				.b_out = (short)((out_configuration & 
						  second_particle_mask)>>16),
#elif CFG_ANICR_THREE
				.a_in  = (short)(in_configuration   & 
						 first_particle_mask),
				.a_out = (short)(out_configuration  & 
						 first_particle_mask),
				.b_in  = (short)((in_configuration  & 
						  second_particle_mask)>>16),
				.b_out = (short)((out_configuration & 
						  second_particle_mask)>>16),
				.c_in  = (short)((in_configuration  & 
						  third_particle_mask)>>32),
				.c_out = (short)((out_configuration & 
						  third_particle_mask)>>32),
#endif
			};		
//			printf("configuration (%lu): "
//#if CFG_ANICR_ONE
//			       "%d %d\n",
//			       j,
//			       current.a_in,
//			       current.a_out);
//#elif CFG_ANICR_TWO
//			"%d %d %d %d\n",
//				j,
//			       current.a_in,
//			       current.b_in,
//			       current.a_out,
//			       current.b_out);
//#elif CFG_ANICR_THREE
//			"%d %d %d %d %d %d\n",
//				j,
//			       current.a_in,
//			       current.b_in,
//			       current.c_in,
//			       current.a_out,
//			       current.b_out,
//			       current.c_out);
//#endif
//			printf("E1 = %d, E2 = %d\n", E1,E2);
//			printf("block_index = %lu\n", block_index);
//			printf("matrix_index = %lu\n", i*out_block_length + j);
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
//			printf("configuration: "
//#if CFG_ANICR_ONE
//			       "%d %d\n",
//			       current.a_in,
//			       current.a_out);
//#elif CFG_ANICR_TWO
//			"%d %d %d %d\n",
//			       current.a_in,
//			       current.b_in,
//			       current.a_out,
//			       current.b_out);
//#elif CFG_ANICR_THREE
//			"%d %d %d %d %d %d\n",
//			       current.a_in,
//			       current.b_in,
//			       current.c_in,
//			       current.a_out,
//			       current.b_out,
//			       current.c_out);
//#endif
			configurations[block_index] = current;
			block_index++;
		}
	}
	char file_name[2048];
	sprintf(file_name,"%s/conn_dE_%d_dM_%d_depth_%d",
		foldername,difference_energy,difference_M,depth_in);
	//printf("Writing %lu number of connections to %s\n",
	//       block_index,file_name);
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
			 size_t max_hash_memory_use)
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
	//printf("Single particle basis:\n");
	//for (size_t i = 0; i<CFG_NUM_SP_STATES0+CFG_NUM_SP_STATES1; i++)
	//{
	//	printf("(%lu): %d %d %d %d %d %d\n",
	//	       i,
	//	       _table_sp_states[i]._n,
	//	       _table_sp_states[i]._l,
	//	       _table_sp_states[i]._j,
	//	       _table_sp_states[i]._m,
	//	       _table_sp_states[i]._nlj,
	//	       _table_sp_states[i]._spi);
	//}
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
	//printf("max_num_blocks = %lu\n",max_num_blocks);
	block_hash.num_buckets = 256;
	max_used = max_hash_memory_use;
	block_hash.buckets = (sp_comb_hash_t**)calloc(block_hash.num_buckets,
						      sizeof(sp_comb_hash_t*));
}

int compute_max_sp_comb_energy()
{
	uint64_t last_combination =
		sp_comb_ind_tables[num_sp_comb_ind_tables-1];
	return combination_energy(last_combination);	
}

void setup_basis_files()
{
	int max_sp_comb_energy = compute_max_sp_comb_energy();
	for (int energy = 0; energy <= max_sp_comb_energy; energy++)
		setup_basis_file(energy);
}

void setup_basis_file(int energy)
{		
	size_t block_start = find_block_start(energy);
	size_t block_length = determine_block_length(block_start,energy);
	basis_state_t *basis_states =
	       	(basis_state_t*)malloc(block_length*sizeof(basis_state_t));
	size_t basis_i;
	//printf("setup_basis_file(%d):\n",energy);
	for (basis_i = 0; basis_i < block_length; basis_i++)
	{
		uint64_t state = sp_comb_ind_tables[basis_i + block_start];
		basis_state_t current_state =
		{
			.a = (short)(state  & first_particle_mask),
#if CFG_ANICR_TWO || CFG_ANICR_THREE
			.b = (short)((state & second_particle_mask)>>16),
#endif
#if CFG_ANICR_THREE
			.c = (short)((state & third_particle_mask)>>32)
#endif
		};
		current_state.a = _table_sp_states[current_state.a]._spi;
#if CFG_ANICR_TWO || CFG_ANICR_THREE
		current_state.b = _table_sp_states[current_state.b]._spi;
#endif
#if CFG_ANICR_THREE
		current_state.c = _table_sp_states[current_state.c]._spi;
#endif
		basis_states[basis_i] = current_state;
//		printf("(%lu): "
//		       "%d "
//#if CFG_ANICR_TWO || CFG_ANICR_THREE
//		       "%d "
//#endif
//#if CFG_ANICR_THREE
//		       "%d "
//#endif
//		       "M = %d E = %d\n",
//		       basis_i,
//		       current_state.a,
//#if CFG_ANICR_TWO || CFG_ANICR_THREE
//		       current_state.b,
//#endif
//#if CFG_ANICR_THREE
//		       current_state.c,
//#endif
//		       combination_M(state),
//		       combination_energy(state));
	}
	char filename[32];
	sprintf(filename,"%s/basis_energy_%d",foldername,energy);
	FILE *basis_file = fopen(filename,"w");
	if (basis_file == NULL)
	{
		fprintf(stderr,"Could not create \"%s\". %s\n",
			filename,strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (fwrite(basis_states,
		   sizeof(basis_state_t),
		   block_length,
		   basis_file)!=block_length)
	{
		fprintf(stderr,"Could not write %lu bytes to file \"%s\"\n",
			sizeof(basis_state_t)*block_length,
			basis_file);
		exit(EXIT_FAILURE);
	}
	fclose(basis_file);
	free(basis_states);
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
	if (block_hash.num_buckets == 0)
		return no_index;
	uint64_t hash = compute_block_hash(difference_energy,
					   difference_M,
					   depth);
	size_t block_index = hash % block_hash.num_buckets;
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
	sp_comb_hash_t *sp_comb_hash = block_hash.buckets[bucket_index];
	if (sp_comb_hash == NULL ||
	    (sp_comb_hash->difference_energy != difference_energy ||
	     sp_comb_hash->difference_M != difference_M ||
	     sp_comb_hash->depth != depth))
	{

		size_t used = block_hash.used_size; 
		if (sp_comb_hash)
		{
			used -= sp_comb_hash->num_buckets * 
				sizeof(configuration_t) * sizeof(size_t);
			free_sp_comb_hash(sp_comb_hash);
		}
		sp_comb_hash =
			setup_sp_comb_basis_and_hash(difference_energy,
						     difference_M,
						     depth);
		if (sp_comb_hash)
		{
			used = used + 
				sp_comb_hash->num_buckets * 
				sizeof(configuration_t) * sizeof(size_t);
			if (used >= max_used)
			{
				for (size_t i = 1; i<block_hash.num_buckets; i++)
				{
					sp_comb_hash_t *hash_to_free =
						block_hash.buckets[bucket_index+i];
					block_hash.buckets[bucket_index+i] = NULL;
					used-=hash_to_free->num_buckets*
						sizeof(configuration_t) * sizeof(size_t);
					free_sp_comb_hash(hash_to_free);
					if (used < max_used)
						break;
				}
			}	
		}
		block_hash.buckets[bucket_index] = sp_comb_hash;
		block_hash.used_size = used;
	}
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
	sprintf(outputfile_filename,
		"%s/index_list_E_in%d_E_out%d_M_in%d_M_out%d_dE%d_dM%d_depth%d",
		foldername,
		energy_in,
		energy_out,
		M_in,
		M_out,
		difference_energy,
		difference_M,
		depth);
	outputfile = fopen(outputfile_filename,"w");
	if (outputfile == NULL)
	{
		fprintf(stderr,"file_name: %s\n",outputfile_filename);
		fprintf(stderr,"Something is terrible wrong, %s\n",
			strerror(errno));
		exit(1);
	}
}

static
void write_buffer_to_file()
{
	if (fwrite(index_triple_buffer,
		   sizeof(index_triple_t),
		   num_triples_in_buffer,
		   outputfile)<num_triples_in_buffer)
		fprintf(stderr,"Could not write %lu triples to %s\n",
			num_triples_in_buffer,
			outputfile_filename);
	num_triples_in_buffer = 0;
}

void close_output_block()
{
	if (num_triples_in_buffer > 0)
		write_buffer_to_file();
	fclose(outputfile);
}

void flip_configuration(configuration_t *configuration)
{
	int tmp;
#if CFG_ANICR_ONE
	tmp = configuration->a_in;
	configuration->a_in = configuration->a_out;
	configuration->a_out = tmp;
#elif CFG_ANICR_TWO
	tmp = configuration->a_in;
	configuration->a_in = configuration->a_out;
	configuration->a_out = tmp;
	tmp = configuration->b_in;
	configuration->b_in = configuration->b_out;
	configuration->b_out = tmp;
#elif CFG_ANICR_THREE
	tmp = configuration->a_in;
	configuration->a_in = configuration->a_out;
	configuration->a_out = tmp;
	tmp = configuration->b_in;
	configuration->b_in = configuration->b_out;
	configuration->b_out = tmp;
	tmp = configuration->c_in;
	configuration->c_in = configuration->c_out;
	configuration->c_out = tmp;
#endif
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
	if (k == no_index)
	{
		//printf("This does indeed happen\n");
		flip_configuration(&current_configuration);
		k = get_configuration_index(current_sp_comb_hash,
					    current_configuration);
	}
	//printf("k = %lu\n",k);
	if (k == no_index)
	{
		fprintf(stderr,"Can't find configuration "
#if CFG_ANICR_ONE
		       "%d %d\n",
		       ain,aout);
#elif CFG_ANICR_TWO
		"%d %d %d %d\n",
			ain,bin,aout,bout);
#elif CFG_ANICR_THREE
		"%d %d %d %d %d %d\n",
			ain,bin,cin,
			aout,bout,cout);
#endif
		exit(EXIT_FAILURE);
	}
	index_triple_buffer[num_triples_in_buffer].i = i;
	index_triple_buffer[num_triples_in_buffer].j = j;
	index_triple_buffer[num_triples_in_buffer].k = 
		k | (sgn<0 ? 0x80000000 : 0);
	num_triples_in_buffer++;
	if (num_triples_in_buffer == INDEX_TRIPLE_BUFFER_SIZE)
		write_buffer_to_file();
}

void write_marker(char* str)
{
	fprintf(outputfile,"%s\n",str);
}

void close_file()
{
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

void finalize_index_files()
{
	close_file();
	free_block_table();
}

#endif
