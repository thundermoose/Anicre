#include "tmp_config.h"
#include "anicr_config.h"
#include "anicr_tables.h"
#include "create.h"
#include "packed_create.h"
#include "accumulate.h"
#include "util.h"
#include "mp_states.h"
#include "indexoutput.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#define MEM_GB(a) (size_t)(a)<<30

void packed_to_int_list(int *list, const uint64_t *packed);
void int_list2_to_packed(uint64_t *packed, const int *list0, const int *list1);

int compare_packed_mp_state(const void *p1, const void *p2)
{
	const uint64_t *s1 = (const uint64_t *) p1;
	const uint64_t *s2 = (const uint64_t *) p2;

	int i;

	for (i = 0; i < CFG_PACK_WORDS; i++)
	{
		if (*s1 < *s2)
			return -1;
		if (*s1 > *s2)
			return 1;

		s1++;
		s2++;
	}

	return 0;
}

int mp_state_in_E(int *in_sp);
int mp_state_in_M(int *in_sp);

int compare_packed_mp_state_E_M(const void *p1, const void *p2)
{
	const uint64_t *s1 = (const uint64_t *) p1;
	const uint64_t *s2 = (const uint64_t *) p2;

	int list1[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];
	int list2[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

	packed_to_int_list(list1, s1);
	packed_to_int_list(list2, s2);

	int E1 = mp_state_in_E(list1);
	int E2 = mp_state_in_E(list2);

	if (E1 != E2)
		return E1 - E2;

	int M1 = mp_state_in_M(list1);
	int M2 = mp_state_in_M(list2);

	if (M1 != M2)
		return M1 - M2;

	int i;

	for (i = 0; i < CFG_PACK_WORDS; i++)
	{
		if (*s1 < *s2)
			return -1;
		if (*s1 > *s2)
			return 1;

		s1++;
		s2++;
	}

	return 0;
}

int sp_comb_E(uint64_t sp);
int sp_comb_M(uint64_t sp);

int compare_sp_comb_E_M(const void *p1, const void *p2)
{
	const uint64_t *s1 = (const uint64_t *) p1;
	const uint64_t *s2 = (const uint64_t *) p2;

	int E1 = sp_comb_E(*s1);
	int E2 = sp_comb_E(*s2);

	if (E1 != E2)
		return E1 - E2;

	int M1 = sp_comb_M(*s1);
	int M2 = sp_comb_M(*s2);

	if (M1 != M2)
		return M1 - M2;

	if (*s1 < *s2)
		return -1;
	if (*s1 > *s2)
		return 1;
	return 0;
}

uint64_t *_mp = NULL;
#if !CFG_CONN_TABLES
double   *_wf = NULL;
#endif

hash_mp_wf *_hashed_mp = NULL;

typedef struct mp_cut_E_M_t
{
	int    _E, _M;
	size_t _start;

	hash_mp_wf *_hashed_mp;
} mp_cut_E_M;

mp_cut_E_M    *_mp_cut_E_M;
size_t     _num_mp_cut_E_M;

uint64_t _lookups = 0;
uint64_t _found = 0;

double   _cur_val;

hash_mp_wf *setup_hash_table(uint64_t *mp,
#if !CFG_CONN_TABLES && !CFG_IND_TABLES
			     double   *wf,
#endif
			     size_t num_mp,
			     int verbose);

#if CFG_CONN_TABLES || CFG_IND_TABLES
size_t sort_mp_by_E_M(size_t num_mp)
{
	/* First, remove all 'other' particles from the states, as we
	 * only care about one kind of particles when building tables.
	 *
	 * Then, sort them by E and M.
	 *
	 * Finally, squeeze out all duplicates.
	 */

	size_t i;

	uint64_t *mp = _mp;

	int list0[CFG_NUM_SP_STATES1];
	memset(list0, 0, sizeof (list0));

	for (i = 0; i < num_mp; i++)
	{
		int list[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

		uint64_t *packed = mp;

		packed_to_int_list(list, packed);

		int_list2_to_packed(packed, list, list0 /* other */);

		mp += CFG_PACK_WORDS;     
	}  
	qsort (_mp, num_mp, sizeof (uint64_t) * CFG_PACK_WORDS,
	       compare_packed_mp_state_E_M);
#if CFG_CONN_TABLES
	printf ("Sorted %zd mp states.\n", num_mp);
#endif
	size_t reduced_num_mp = 1;

	uint64_t *mp_in  = _mp + CFG_PACK_WORDS;
	uint64_t *mp_out = _mp;

	for (i = 1; i < num_mp; i++)
	{
		int diff = 0;
		int j;

		for (j = 0; j < CFG_PACK_WORDS; j++)      
			diff |= (mp_in[j] != mp_out[j]);

		if (diff)
		{
			reduced_num_mp++;
			mp_out += CFG_PACK_WORDS;

			for (j = 0; j < CFG_PACK_WORDS; j++)
				mp_out[j] = mp_in[j];
		}

		mp_in += CFG_PACK_WORDS;
	}

	mp_out += CFG_PACK_WORDS;
#if CFG_CONN_TABLES
	printf ("Reduced %zd mp states.\n", reduced_num_mp);
#endif
	_num_mp_cut_E_M = 0;

	mp_cut_E_M tmp_E_M;

	tmp_E_M._E = -1;
	tmp_E_M._M = 0; /* does not matter */

	mp = _mp;

	for (i = 0; i < reduced_num_mp; i++)
	{
		int list[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

		uint64_t *packed = mp;

		packed_to_int_list(list, packed);

#if 0
		{
			size_t i1;

			for (i1 = 0; i1 < CFG_NUM_SP_STATES0; i1++)
			{
				printf (" %3d", list[i1]);
			}
			printf ("\n");
		}
#endif

		int E = mp_state_in_E(list);
		int M = mp_state_in_M(list);

		if (E != tmp_E_M._E || M != tmp_E_M._M)
		{
			tmp_E_M._E = E;
			tmp_E_M._M = M;
			_num_mp_cut_E_M++;
		}

		mp += CFG_PACK_WORDS;      
	}

	_mp_cut_E_M =
		(mp_cut_E_M *) malloc ((_num_mp_cut_E_M + 1) * sizeof (mp_cut_E_M));

	if (!_mp_cut_E_M)
	{
		fprintf (stderr, "Memory allocation error (%zd bytes).\n",
			 _num_mp_cut_E_M * sizeof (mp_cut_E_M));     
	}

	tmp_E_M._E = -1;
	tmp_E_M._M = 0; /* does not matter */

	size_t cut_E_M = 0;

	mp = _mp;

	for (i = 0; i < reduced_num_mp; i++)
	{
		int list[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

		uint64_t *packed = mp;

		packed_to_int_list(list, packed);

		int E = mp_state_in_E(list);
		int M = mp_state_in_M(list);

		if (E != tmp_E_M._E || M != tmp_E_M._M)
		{
			_mp_cut_E_M[cut_E_M]._E = E;
			_mp_cut_E_M[cut_E_M]._M = M;
			_mp_cut_E_M[cut_E_M]._start = i;
			_mp_cut_E_M[cut_E_M]._hashed_mp = NULL;

			tmp_E_M._E = E;
			tmp_E_M._M = M;
			cut_E_M++;
		}

		/*
		   printf ("%2d %3d : ", E, M);

		   int j;

		   for (j = 0; j < CFG_NUM_SP_STATES0; j++)
		   {
		   printf (" %5d", list[j]);

		   }

		   printf ("\n");
		 */
		mp += CFG_PACK_WORDS;
	}

	_mp_cut_E_M[cut_E_M]._start = reduced_num_mp;
#if CFG_CONN_TABLES
	printf ("%zd mp state E-M pairs\n", _num_mp_cut_E_M);
#endif
	for (i = 0; i < _num_mp_cut_E_M; i++)
	{
		size_t mp_states = _mp_cut_E_M[i+1]._start - _mp_cut_E_M[i]._start;

		_mp_cut_E_M[i]._hashed_mp =
			setup_hash_table(_mp + _mp_cut_E_M[i]._start * CFG_PACK_WORDS,
					 mp_states,
					 0);
#if CFG_CONN_TABLES
		printf (TABLE_PREFIX "_" "E_M_PAIR  %2d %3d : %10zd\n",
			_mp_cut_E_M[i]._E,
			_mp_cut_E_M[i]._M,
			mp_states);
#endif
	}

	return reduced_num_mp;
}
#endif

uint64_t  *_sp_comb = NULL;
size_t _num_sp_comb = 0;

typedef struct sp_comb_cut_E_M_t
{
	int    _E, _M;
	size_t _start;

	/* hash_sp_comb_wf *_hashed_sp_comb; */
} sp_comb_cut_E_M;

sp_comb_cut_E_M    *_sp_comb_cut_E_M;
size_t          _num_sp_comb_cut_E_M;

#if CFG_CONN_TABLES || CFG_IND_TABLES
void find_sp_comb(size_t num_mp)
{
	/* For each mp state, we can find 
	 * 1: CFG_NUM_SP_STATES0
	 * 2: CFG_NUM_SP_STATES0 * (CFG_NUM_SP_STATES0 - 1) / 2
	 * 3: CFG_NUM_SP_STATES0 * (CFG_NUM_SP_STATES0 - 1) / 2 *
	 *    (CFG_NUM_SP_STATES0 - 2) / 3
	 * combinations.  Let's simply allocate for them all,
	 * and sort at the end...
	 */

	size_t max_num_sp_comb = num_mp * CFG_NUM_SP_STATES0;
#if CFG_ANICR_TWO || CFG_ANICR_THREE
	max_num_sp_comb = ((max_num_sp_comb) * (CFG_NUM_SP_STATES0 - 1)) / 2;
#endif
#if CFG_ANICR_THREE
	max_num_sp_comb = ((max_num_sp_comb) * (CFG_NUM_SP_STATES0 - 2)) / 3;
#endif
	printf("max_num_sp_comb: %ld\n",max_num_sp_comb);
	_sp_comb = (uint64_t *) calloc (max_num_sp_comb, sizeof (uint64_t));
	printf("_sp_comb = %p\n",_sp_comb);
	if (!_sp_comb)
	{
		fprintf (stderr, "Memory allocation error (%zd bytes).\n",
			 max_num_sp_comb * sizeof (uint64_t));
	}

	size_t i;

	uint64_t *mp = _mp;
	uint64_t *sp_comb = _sp_comb;

	for (i = 0; i < num_mp; i++)
	{
		int list[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

		uint64_t *packed = mp;

		packed_to_int_list(list, packed);

		size_t i1;
		uint64_t comb1;
#if CFG_ANICR_TWO || CFG_ANICR_THREE
		size_t i2;
		uint64_t comb2;
#endif
#if CFG_ANICR_THREE
		size_t i3;
		uint64_t comb3;
#endif
		uint64_t comb;

		for (i1 = 0; i1 < CFG_NUM_SP_STATES0; i1++)
		{
			comb1 = ((uint64_t) list[i1]) << 0;

#if CFG_ANICR_TWO || CFG_ANICR_THREE
			for (i2 = i1+1; i2 < CFG_NUM_SP_STATES0; i2++)
			{
				comb2 = comb1 | (((uint64_t) list[i2]) << 16);
#endif

#if CFG_ANICR_THREE
				for (i3 = i2+1; i3 < CFG_NUM_SP_STATES0; i3++)
				{
					comb3 = comb2 | (((uint64_t) list[i3]) << 32);
					comb = comb3;

#elif CFG_ANICR_TWO
					comb = comb2;
#else
					comb = comb1;
#endif
					*(sp_comb++) = comb;
					printf("COMB: %0.16lx\n",comb);
#if CFG_ANICR_THREE
				}
#endif
#if CFG_ANICR_TWO || CFG_ANICR_THREE
			}
#endif
		}

		mp += CFG_PACK_WORDS;
	}
	printf ("%zd sp combinations\n", max_num_sp_comb);

	qsort (_sp_comb, max_num_sp_comb, sizeof (uint64_t),
	       compare_sp_comb_E_M);

	printf ("Sorted %zd sp combinations.\n", num_mp);

	size_t reduced_num_sp_comb = 1;

	uint64_t *sp_comb_in  = _sp_comb + 1;
	uint64_t *sp_comb_out = _sp_comb;

	for (i = 1; i < max_num_sp_comb; i++)
	{
		int diff = 0;

		diff = (*sp_comb_in != *sp_comb_out);

		if (diff)
		{
			reduced_num_sp_comb++;
			sp_comb_out++;

			*sp_comb_out = *sp_comb_in;
		}

		sp_comb_in++;
	}

	sp_comb_out++;

	printf ("Reduced %zd sp combinations.\n", reduced_num_sp_comb);
#if CFG_IND_TABLES
	sp_comb_ind_tables = (uint64_t*)malloc(sizeof(uint64_t)*reduced_num_sp_comb);
	memcpy(sp_comb_ind_tables,_sp_comb,sizeof(uint64_t)*reduced_num_sp_comb);
	num_sp_comb_ind_tables = reduced_num_sp_comb;
	for (i = 0; i<reduced_num_sp_comb; i++){
#if CFG_ANICR_ONE
		printf("comb[%ld] = %ld\n",
		       i,_sp_comb[i]&0xFFFF);
#elif CFG_ANICR_TWO
		printf("comb[%ld] = %ld %ld\n",
		       i,_sp_comb[i]&0xFFFF,
		       (_sp_comb[i]>>16)&0xFFFF);
#elif CFG_ANICR_THREE
		printf("comb[%ld] = %ld %ld %ld\n",
		       i,_sp_comb[i]&0xFFFF,
		       (_sp_comb[i]>>16)&0xFFFF,
		       (_sp_comb[i]>>32)&0xFFFF);
#endif
		printf("comb[%ld] = 0x%lx\n",i,_sp_comb[i]);
	}
#endif
	//#if !CFG_IND_TABLES
	_num_sp_comb_cut_E_M = 0;

	mp_cut_E_M tmp_E_M;

	tmp_E_M._E = -1;
	tmp_E_M._M = 0; /* does not matter */

	sp_comb = _sp_comb;

	for (i = 0; i < reduced_num_sp_comb; i++)
	{
		int E = sp_comb_E(*sp_comb);
		int M = sp_comb_M(*sp_comb);

		if (E != tmp_E_M._E || M != tmp_E_M._M)
		{
			tmp_E_M._E = E;
			tmp_E_M._M = M;
			_num_sp_comb_cut_E_M++;
		}

		sp_comb++;
	}

	_sp_comb_cut_E_M =
		(sp_comb_cut_E_M *) malloc ((_num_sp_comb_cut_E_M + 1) * sizeof (sp_comb_cut_E_M));

	if (!_sp_comb_cut_E_M)
	{
		fprintf (stderr, "Memory allocation error (%zd bytes).\n",
			 _num_sp_comb_cut_E_M * sizeof (sp_comb_cut_E_M));     
	}

	tmp_E_M._E = -1;
	tmp_E_M._M = 0; /* does not matter */

	size_t cut_E_M = 0;

	sp_comb = _sp_comb;

	for (i = 0; i < reduced_num_sp_comb; i++)
	{
		int E = sp_comb_E(*sp_comb);
		int M = sp_comb_M(*sp_comb);

		if (E != tmp_E_M._E || M != tmp_E_M._M)
		{

			_sp_comb_cut_E_M[cut_E_M]._E = E;
			_sp_comb_cut_E_M[cut_E_M]._M = M;
			_sp_comb_cut_E_M[cut_E_M]._start = i;

			tmp_E_M._E = E;
			tmp_E_M._M = M;
			cut_E_M++;
		}

		sp_comb++;
	}
	printf("cut_E_M: %ld\n",cut_E_M);
	printf("_num_sp_comb_cut_E_M: %ld\n",_num_sp_comb_cut_E_M);
	_sp_comb_cut_E_M[cut_E_M]._start = reduced_num_sp_comb;

	printf ("%zd sp comb E-M pairs\n", _num_sp_comb_cut_E_M);

	for (i = 0; i < _num_sp_comb_cut_E_M; i++)
	{
		size_t sp_combs =
			_sp_comb_cut_E_M[i+1]._start - _sp_comb_cut_E_M[i]._start;

		/*
		   _sp_comb_cut_E_M[i]._hashed_sp_comb =
		   setup_hash_table(_sp_comb + _sp_comb_cut_E_M[i]._start * CFG_PACK_WORDS,
		   sp_combs,
		   0);
		 */
#if CFG_CONN_TABLES
		printf("i = %ld\n",i);
		printf (TABLE_PREFIX "_" "V_E_M_PAIR  %2d %3d : %10zd\n",
			_sp_comb_cut_E_M[i]._E,
			_sp_comb_cut_E_M[i]._M,
			sp_combs);
#endif
	}
	//#endif
}
#endif

hash_mp_wf *setup_hash_table(uint64_t *mp,
#if !CFG_CONN_TABLES && !CFG_IND_TABLES
			     double   *wf,
#endif
			     size_t num_mp,
			     int verbose)
{
	hash_mp_wf *hashed_mp = (hash_mp_wf *) malloc (sizeof (hash_mp_wf));

	if (!hashed_mp)
	{
		fprintf (stderr, "Memory allocation error (%zd bytes).\n",
			 sizeof (hash_mp_wf));
		exit(1);
	}

	for (hashed_mp->_hash_mask = 1; 
	     hashed_mp->_hash_mask < num_mp * 2;
	     hashed_mp->_hash_mask <<= 1)
		;

	size_t hashed_mp_sz = sizeof (hash_mp_wf_item) * hashed_mp->_hash_mask;

	hashed_mp->_hash_mask -= 1;

	/* printf ("%"PRIu64" %zd\n",_hash_mask, hashed_mp_sz); */

	hashed_mp->_hashed = (hash_mp_wf_item *) malloc (hashed_mp_sz);

	if (!hashed_mp->_hashed)
	{
		fprintf (stderr, "Memory allocation error (%zd bytes).\n", hashed_mp_sz);
		exit(1);
	}

	memset (hashed_mp->_hashed, 0, hashed_mp_sz);

	size_t i;

	uint64_t max_coll = 0, sum_coll = 0;

	for (i = 0; i < num_mp; i++)
	{
		uint64_t x = packed_hash_key(mp);

		x ^= x >> 32;

		uint64_t j = x & hashed_mp->_hash_mask;

		uint64_t coll = 0;

		while (hashed_mp->_hashed[j]._mp[0] != 0)
		{
			j = (j + 1) & hashed_mp->_hash_mask;
			coll++;
		}

		if (coll > max_coll)
			max_coll = coll;
		sum_coll += coll;

		uint32_t k;

		for (k = 0; k < CFG_PACK_WORDS; k++)
		{
			hashed_mp->_hashed[j]._mp[k] = mp[k];
		}
#if !CFG_CONN_TABLES && !CFG_IND_TABLES
		for (k = 0; k < CFG_WAVEFCNS; k++)
		{
			hashed_mp->_hashed[j]._wf[k] = wf[k];
		}
#else
		hashed_mp->_hashed[j]._index = i;
#endif

		mp += CFG_PACK_WORDS;
#if !CFG_CONN_TABLES && !CFG_IND_TABLES
		wf += CFG_WAVEFCNS;
#endif
	}

	if (verbose)
		printf ("Hash: %"PRIu64" entries (%.1f), "
			"avg coll = %.2f, max coll = %"PRIu64"\n",
			hashed_mp->_hash_mask + 1,
			(double) num_mp / (double) (hashed_mp->_hash_mask + 1),
			(double) sum_coll / (double) num_mp, max_coll);

	return hashed_mp;
}

extern double one_coeff[CFG_NUM_SP_STATES][CFG_NUM_SP_STATES];

//#if CFG_IND_TABLES
//void setup_basis_files()
//{
//	size_t cut_i;
//	for (cut_i = 0; cut_i < _num_mp_cut_E_M; cut_i++)
//	{
//		mp_cut_E_M *cut = _mp_cut_E_M + cut_i;
//		setup_basis_file(cut->_E);
//	}
//}
//#endif

void process_mp_cut(mp_cut_E_M *cut_ini,
		    mp_cut_E_M *cut_fin,
		    size_t mp_states,
		    size_t *tot_ini_states)
{

	size_t i;
	static size_t prev_found = 0;
	int diff_E = cut_fin->_E - cut_ini->_E;
	int diff_M = cut_fin->_M - cut_ini->_M;

	int max_depth =
		cut_ini->_E;


#if CFG_ANICR_THREE
	if (diff_M != 0)
		return;
#endif

	int depth;

	for (depth = 0; depth <= max_depth; depth++)
	{
		uint64_t *mp = _mp + cut_ini->_start * CFG_PACK_WORDS;
#if CFG_IND_TABLES
		new_output_block(cut_ini->_E,cut_fin->_E,
				 cut_ini->_M,cut_fin->_M,
				 diff_E,diff_M,depth);
#endif
		for (i = 0; i < mp_states; i++)
		{
#if CFG_IND_TABLES
			indin = i;
#endif
			annihilate_packed_states(mp,
						 diff_E & 1, diff_M, diff_E,
						 depth);

			mp += CFG_PACK_WORDS;

		}
#if CFG_IND_TABLES
		close_output_block();
#endif
#if CFG_CONN_TABLES
		printf (TABLE_PREFIX "_" "CONN "
			"%2d %3d  ->  %2d %3d  : %2d :  "
			"dE=%2d dM=%3d  : %10zd %10zd : "
			"%10" PRIu64 "\n",
			cut_ini->_E,
			cut_ini->_M,
			cut_fin->_E,
			cut_fin->_M,
			depth,
			diff_E, diff_M,
			mp_states,
			(cut_fin+1)->_start - cut_fin->_start,
			_found - prev_found);
#endif
		prev_found = _found;

		*tot_ini_states += mp_states;
	}
}

int main(int argc, char *argv[])
{
	size_t num_mp = CFG_NUM_MP_STATES;
	/* size_t num_sp = CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1; */

	assert(sizeof (uint64_t) == sizeof (double));

#if CFG_CONN_TABLES
#if (CFG_ANICR_TWO && CFG_NUM_SP_STATES0<2) || (CFG_ANICR_THREE && CFG_NUM_SP_STATES0<3)
	printf("NO_"TABLE_PREFIX"_INTERACTIONS\n");
	return 0;
#endif
#elif CFG_IND_TABLES
#if (CFG_ANICR_TWO && CFG_NUM_SP_STATES0<2) || (CFG_ANICR_THREE && CFG_NUM_SP_STATES0<3)
	return 0;
#endif
#endif

#if CFG_CONN_TABLES
	printf ("CFG_MAX_E: %d\n", CFG_MAX_SUM_E);
	printf ("CFG_M:     %d\n", CFG_SUM_M);
	printf ("CFG_P:     %d\n", CFG_PARITY_INITIAL);
#endif

	size_t mp_sz = sizeof (uint64_t) * (CFG_PACK_WORDS) * num_mp;
#if !CFG_CONN_TABLES && !CFG_IND_TABLES
	size_t wf_sz = sizeof (double)   * (CFG_WAVEFCNS) * num_mp;
#endif

	_mp = (uint64_t *) malloc (mp_sz);
#if !CFG_CONN_TABLES && !CFG_IND_TABLES
	_wf = (double *)   malloc (wf_sz);
#endif

	if (!_mp)
	{
		fprintf (stderr, "Memory allocation error (mp, %zd bytes).\n", mp_sz);
		exit(1);
	}

#if !CFG_CONN_TABLES && !CFG_IND_TABLES
	if (!_wf)
	{
		fprintf (stderr, "Memory allocation error (wf, %zd bytes).\n", wf_sz);
		exit(1);
	}
#endif

	char filename_states_all[256];

	sprintf (filename_states_all, "states_all_%s_orig.bin", CFG_MP_STATES_FR);

	int fd = open (filename_states_all, O_RDONLY);

	if (fd == -1)
	{
		perror("open");
		exit(1);
	}

	full_read (fd, _mp, mp_sz);

	close (fd);

	printf ("Read %zd mp states.\n", num_mp);

#if !CFG_CONN_TABLES && !CFG_IND_TABLES
	fd = open ("wavefcn_all_orig.bin", O_RDONLY);

	if (fd == -1)
	{
		perror("open");
		exit(1);
	}

	full_read (fd, _wf, wf_sz);

	close (fd);

	printf ("Read %zd wf coeffs.\n", num_mp);
#endif

#if CFG_CONN_TABLES || CFG_IND_TABLES
	num_mp = sort_mp_by_E_M(num_mp);

	find_sp_comb(num_mp);
#endif

#if CFG_ANICR_ONE  //DS - unused variables
	(void)argv;
	(void)argc;
#endif


#if !CFG_CONN_TABLES /* lets not even set it up... */
	_hashed_mp = setup_hash_table(_mp,
#if !CFG_CONN_TABLES && !CFG_IND_TABLES
				      _wf,
#endif
				      num_mp,
				      1);
#endif

#if 0 
	/* It turns out that lookup is ~ 20 % faster with original states
	 * in original antoine order...
	 */

	qsort (_mp, num_mp, sizeof (uint64_t) * (CFG_PACK_WORDS + CFG_WAVEFCNS),
	       compare_packed_mp_state);

	printf ("Sorted %zd mp states.\n", num_mp);
#endif
	//#endif  //DS
	ammend_tables();

	int packed = 0;

	if (argc > 1 && strcmp(argv[1],"--packed") == 0)
		packed = 1;

	(void) packed;
#if !CFG_CONN_TABLES && !CFG_IND_TABLES

#if CFG_ANICR_TWO  //DS
	prepare_accumulate();

	prepare_nlj();

	alloc_accumulate();
#endif
	size_t i;

#if DEBUG_ANICR || 1
	/* print the mp basis*/
	uint64_t *mmp = _mp;
	printf("The mp basis\n");
	for (i = 0; i < num_mp; i++){
		int list[CFG_NUM_SP_STATES0+CFG_NUM_SP_STATES1];
		packed_to_int_list(list,mmp);
		int j;
		printf("(%ld): ",i);
		for (j = 0; j<CFG_NUM_SP_STATES0+CFG_NUM_SP_STATES1; j++){
			printf("%d ",list[j]);
		}
		printf("\n");
		mmp+=CFG_PACK_WORDS;
	}

#endif

	uint64_t *mp = _mp;
	double   *wf = _wf;
#if CFG_IND_TABLES
	find_sp_comb(num_mp);

	initiate_index_file(num_mp);
	//i = 27;
#endif
	for (i = 0; i < num_mp; i++)
	{
#if CFG_IND_TABLES
		indin = i;
#endif
		_cur_val = wf[0];

		if (packed)
		{
			packed_annihilate_states(mp);
		}
		else
		{
			//annihilate_states(mp + CFG_NUM_SP_STATES0, mp); //DS
			annihilate_packed_states(mp);
		}

		mp += CFG_PACK_WORDS;
		wf += CFG_WAVEFCNS;

		if (i % 10000 == 0)
		{
			printf ("anicr %zd / %zd\r", i, num_mp);
			fflush (stdout);
		}

	}

#if CFG_IND_TABLES
	close_file();
#endif  
	printf ("Annihilated-created for %zd mp states.\n", num_mp);

	printf ("Found %"PRIu64"/%"PRIu64".\n", _found, _lookups);

#if CFG_ANICR_ONE
	couple_accumulate(); 
#elif CFG_ANICR_TWO
	couple_accumulate_2();
	write_nlj();
#endif
#endif

#if CFG_CONN_TABLES || CFG_IND_TABLES
#if CFG_IND_TABLES
	initiate_index_file(num_mp,
			    MEM_GB(80));
	{
		// Lists all cuts
		size_t cut_i;
		char filename[256];
		sprintf(filename,"%s_cut_out_list",CFG_ANICR_IDENT);
		FILE* cut_out_list = fopen(filename,"w");
		sprintf(filename,"%s_basis_out_list",CFG_ANICR_IDENT);
		FILE* basis_list = fopen(filename,"w");
		fprintf(basis_list,"Num part: %ld\n",CFG_NUM_SP_STATES0);
		fprintf(cut_out_list,
			"E\tM\tstart\tend\n");
		size_t mps_i = 0;
		size_t num_states  = 0;
		for (cut_i = 0; cut_i < _num_mp_cut_E_M; cut_i++)
		{
			num_states += 
				CFG_NUM_SP_STATES0*
				(_mp_cut_E_M[cut_i+1]._start-_mp_cut_E_M[cut_i]._start);
		}
		int *states = (int*)malloc(num_states*sizeof(int));
		size_t state_i = 0;
		for (cut_i = 0; cut_i < _num_mp_cut_E_M; cut_i++)
		{
			fprintf(cut_out_list,
				"%d\t%d\t%d\t%d\n",
				_mp_cut_E_M[cut_i]._E,
				_mp_cut_E_M[cut_i]._M,
				_mp_cut_E_M[cut_i]._start,
				_mp_cut_E_M[cut_i+1]._start);
			uint64_t *mp = _mp + _mp_cut_E_M[cut_i]._start * CFG_PACK_WORDS;
			size_t i;
			for (i = 0; i<_mp_cut_E_M[cut_i+1]._start-_mp_cut_E_M[cut_i]._start; i++){
				int list[CFG_NUM_SP_STATES0+CFG_NUM_SP_STATES1];
				packed_to_int_list(list,mp);
				size_t j;
				fprintf(basis_list,"(%ld): ",mps_i++);
				for (j = 0; j<CFG_NUM_SP_STATES0; j++){
					fprintf(basis_list,"%d ",_table_sp_states[list[j]]._spi);
					states[state_i++] = _table_sp_states[list[j]]._spi;
				}
				fprintf(basis_list,"\n");
				mp+=CFG_PACK_WORDS;
			}
		}
		fclose(cut_out_list);
		fclose(basis_list);
		sprintf(filename,"%s_basis_out_list.bin",CFG_ANICR_IDENT);
		FILE* basis_list_bin = fopen(filename,"w");
		if (basis_list_bin == NULL)
		{
			fprintf(stderr, "Could not open file %s\n",filename);
		}
		else if (fwrite(states,
				num_states,
				sizeof(int),
				basis_list_bin) < num_states)
		{
			fprintf(stderr, "Could not write basis to %s\n",filename);
		}
		if (basis_list_bin)
			fclose(basis_list_bin);
		free(states);
	}
#endif
#if CFG_CONN_TABLES
	printf("_num_mp_cut_E_M = %lu\n",
	       _num_mp_cut_E_M);
#endif

#if CFG_IND_TABLES
	setup_basis_files();	
#endif


	size_t cut_ini_i;
	size_t cut_fin_i;

	size_t tot_ini_states = 0;

	uint64_t prev_found = 0;
	if (argc == 3)
	{
		cut_ini_i = atoll(argv[1]);
		cut_fin_i = atoll(argv[2]);
		mp_cut_E_M *cut_ini = _mp_cut_E_M + cut_ini_i;
		mp_cut_E_M *cut_fin = _mp_cut_E_M + cut_fin_i;
		size_t mp_states = (cut_ini+1)->_start - cut_ini->_start;
		_hashed_mp = cut_fin->_hashed_mp;
		process_mp_cut(cut_ini,
			       cut_fin,
			       mp_states,
			       &tot_ini_states);
	}
	else
	{
		for (cut_ini_i = 0; cut_ini_i < _num_mp_cut_E_M; cut_ini_i++)
		{
			mp_cut_E_M *cut_ini = _mp_cut_E_M + cut_ini_i;

			for (cut_fin_i = 0; cut_fin_i < _num_mp_cut_E_M; cut_fin_i++)
				//for (cut_fin_i = 0; cut_fin_i <= cut_ini_i; cut_fin_i++)
			{
				mp_cut_E_M *cut_fin = _mp_cut_E_M + cut_fin_i;

				size_t mp_states = (cut_ini+1)->_start - cut_ini->_start;

				_hashed_mp = cut_fin->_hashed_mp;

				process_mp_cut(cut_ini,
					       cut_fin,
					       mp_states,
					       &tot_ini_states);
				fprintf (stderr,
					 "anicr %zd : %zd / %zd   \r",
					 cut_ini_i+1, cut_fin_i+1, _num_mp_cut_E_M);
				fflush (stderr);
			}

		}
	}
	printf ("Annihilated-created for %zd mp states "
		"in %zd * %zd E-M combinations.\n",
		tot_ini_states,
		_num_mp_cut_E_M, _num_mp_cut_E_M);

	printf ("Found mb state in hashtable %"PRIu64" of %"PRIu64" lookups.\n", _found, _lookups);
#if CFG_IND_TABLES
	finalize_index_files();
#endif 
#endif

	return 0;
}

