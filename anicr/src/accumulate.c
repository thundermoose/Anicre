
#include "accumulate.h"
#include "util.h"

#include "anicr_tables.h"
#include "anicr_config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <limits.h>

#include "tmp_config.h"

uint32_t *_jm_pairs = NULL;

void prepare_accumulate()
{
  /* Read in the jm pairs info. */

  size_t sz_jm_pairs = sizeof (uint32_t) * CFG_JM_PAIRS;

  _jm_pairs = (uint32_t*) malloc (sz_jm_pairs);

  if (!_jm_pairs)
    {
      fprintf (stderr, "Memory allocation error (%zd bytes).\n", sz_jm_pairs);
      exit(1);
    }

  int fd = open ("jm_pairs.bin", O_RDONLY);

  if (fd == -1)
    {
      perror("open");
      exit(1);
    }

  full_read (fd, _jm_pairs, sz_jm_pairs);

  close (fd);
  /*
  size_t a_i;

  for (a_i = 0; a_i < CFG_JM_PAIRS; a_i++)
    {
      printf ("%5zd: %3d %3d\n",
	      a_i, _jm_pairs[a_i] & 0xffff, _jm_pairs[a_i] >> 16);
    }
  */
  printf ("Read %zd jm pairs states.\n", (size_t) CFG_JM_PAIRS);
}

uint64_t acc_hash_key(uint64_t key)
{
  uint64_t x = 0, y = 0;

  y = key;
  x = key;

#define _lcga 6364136223846793005ll
#define _lcgc 1442695040888963407ll

  x = x * _lcga + _lcgc;
  
  x = x ^ (x << 35);
  x = x ^ (x >> 4);
  x = x ^ (x << 17);
  /*
  x = x ^ (x >> 35);
  x = x ^ (x << 4);
  x = x ^ (x >> 17);
  */
  /*
  printf ("%016"PRIx64":%016"PRIx64" -> %016"PRIx64"\n",
          key[0], key[1], x);
  */
  return x ^ y;
}

#if ACC_TABLE
double *_accumulate;
#endif

typedef struct accumulate_hash_item_t
{
  uint64_t _key;
  double   _value;
} accumulate_hash_item;

accumulate_hash_item *_acc_hash;
uint64_t              _acc_hash_mask = 0;

typedef struct jm_pair_info_t
{
  char _j1, _m1;
  char _j2, _m2;
} jm_pair_info;

typedef struct jm_pair_info_sort_t
{
  jm_pair_info _info;

  int _parity;

  uint32_t _pair;
} jm_pair_info_sort;
/*
typedef struct jm_pair_subgroup_t
{
  uint32_t *_pairs;
  uint32_t  _num;
} jm_pair_subgroup;
*/
typedef struct jm_pair_group_t
{
  jm_pair_info _info;

  uint32_t *_pairs;
  uint32_t  _num;

} jm_pair_group;

typedef struct jm_pair_group2_t
{
  jm_pair_info _info;

  uint32_t *_pairs;
  uint32_t  _num[2];

} jm_pair_group2;

uint32_t *_jm_pairs_group2_list = NULL;
  /*
jm_pair_subgroup *_jm_pair_subgroups = NULL;
size_t        _num_jm_pair_subgroups = 0;
  */
jm_pair_group2 *_jm_pair_group2s = NULL;
size_t      _num_jm_pair_group2s = 0;

uint32_t *_jm_pairs_group_list = NULL;

jm_pair_group  *_jm_pair_groups = NULL;
size_t      _num_jm_pair_groups = 0;

jm_pair_group **_summ_parity_jm_pair_groups = NULL;
int             _summ_parity_jm_pair_groups_min_sum_m;

#define COMPARE_RET_DIFF(x,y) {if ((x) != (y)) return ((x) < (y)) ? -1 : 1; }

int compare_jm_pair_info_sort_jmjm(const void *p1, const void *p2)
{
  const jm_pair_info_sort *s1 = (const jm_pair_info_sort *) p1;
  const jm_pair_info_sort *s2 = (const jm_pair_info_sort *) p2;

  COMPARE_RET_DIFF(s1->_info._j1, s2->_info._j1);
  COMPARE_RET_DIFF(s1->_info._m1, s2->_info._m1);
  COMPARE_RET_DIFF(s1->_info._j2, s2->_info._j2);
  COMPARE_RET_DIFF(s1->_info._m2, s2->_info._m2);

  return 0;
}
    
int compare_jm_pair_info_sort_parity(const void *p1, const void *p2)
{
  const jm_pair_info_sort *s1 = (const jm_pair_info_sort *) p1;
  const jm_pair_info_sort *s2 = (const jm_pair_info_sort *) p2;

  COMPARE_RET_DIFF(s1->_parity, s2->_parity);

  return 0;
}

int compare_jm_pair_info_sort_summ_parity(const void *p1, const void *p2)
{
  const jm_pair_info_sort *s1 = (const jm_pair_info_sort *) p1;
  const jm_pair_info_sort *s2 = (const jm_pair_info_sort *) p2;

  COMPARE_RET_DIFF(s1->_info._m1 + s1->_info._m2,
		   s2->_info._m1 + s2->_info._m2);
  COMPARE_RET_DIFF(s1->_parity, s2->_parity);

  return 0;
}

int compare_jm_pair_info_sort(const void *p1, const void *p2)
{
  int ret;

  if ((ret = compare_jm_pair_info_sort_jmjm(p1, p2)))
    return ret;
  
  return compare_jm_pair_info_sort_parity(p1, p2);
}

int compare_jm_pair_info_sort_major_summ_parity(const void *p1, const void *p2)
{
  int ret;

  if ((ret = compare_jm_pair_info_sort_summ_parity(p1, p2)))
    return ret;
  
  return compare_jm_pair_info_sort_jmjm(p1, p2);
}

void alloc_accumulate()
{
#if ACC_TABLE
  size_t num_accum;

#if ANICR2
  num_accum = CFG_TOT_FIRST_SCND * CFG_TOT_FIRST_SCND;
#else
  num_accum = CFG_NUM_SP_STATES * CFG_NUM_SP_STATES;
#endif

  size_t accum_sz = sizeof (double) * num_accum;

  _accumulate = (double *) malloc (accum_sz);

  if (!_accumulate)
    {
      fprintf (stderr, "Memory allocation error (%zd bytes).\n", accum_sz);
      exit(1);
    }

  printf ("Allocated %zd items for accumulation.\n", num_accum);

  memset (_accumulate, 0, accum_sz); 
#endif

  /* The reduced version. */

  /* List of the pairs sorted by j,m for the contained states. */

  size_t sz_jmpis = sizeof (jm_pair_info_sort) * CFG_JM_PAIRS;

  jm_pair_info_sort *jmpis = (jm_pair_info_sort *) malloc (sz_jmpis);

  if (!jmpis)
    {
      fprintf (stderr, "Memory allocation error (%zd bytes).\n",
	       sz_jmpis);
      exit(1);
    }

  size_t i;

  int min_sum_m = INT_MAX;
  int max_sum_m = INT_MIN;

  for (i = 0; i < CFG_JM_PAIRS; i++)
    {
      uint32_t pair = _jm_pairs[i];

      uint32_t sp_1 = pair & 0x0000ffff;
      uint32_t sp_2 = pair >> 16;

      int l_1 = _table_sp_states[sp_1]._l;
      int l_2 = _table_sp_states[sp_2]._l;

      int j_1 = _table_sp_states[sp_1]._j;
      int j_2 = _table_sp_states[sp_2]._j;

      int m_1 = _table_sp_states[sp_1]._m;
      int m_2 = _table_sp_states[sp_2]._m;

      jmpis[i]._info._j1 = (char) j_1;
      jmpis[i]._info._j2 = (char) j_2;
      jmpis[i]._info._m1 = (char) m_1;
      jmpis[i]._info._m2 = (char) m_2;

      jmpis[i]._parity = (l_1 + l_2) & 1;

      jmpis[i]._pair = pair;

      int sum_m = m_1 + m_2;

      if (sum_m < min_sum_m)
	min_sum_m = sum_m;
      if (sum_m > max_sum_m)
	max_sum_m = sum_m;
    }

  qsort (jmpis, CFG_JM_PAIRS, sizeof (jm_pair_info_sort),
	 compare_jm_pair_info_sort);

  /* Count number of unique jms. */

  size_t num_jm_infos = 1;
  /*size_t num_parity_infos = 1;*/

  for (i = 1; i < CFG_JM_PAIRS; i++)
    {
      int ret_jmjm = compare_jm_pair_info_sort_jmjm(&jmpis[i-1], &jmpis[i]);

      if (ret_jmjm)
	{
	  num_jm_infos++;
	  /*num_parity_infos++;*/
	}
      else
	{
	  /*
	  int ret_parity =
	    compare_jm_pair_info_sort_parity(&jmpis[i-1], &jmpis[i]);

	  if (ret_parity)
	    num_parity_infos++;
	  */
	}
    }

  printf ("%zd jm-jm combinations"/*", %zd parity subgroups"*/".\n",
	  num_jm_infos/*, num_parity_infos*/);

  /* _num_jm_pair_subgroups = num_parity_infos; */
  _num_jm_pair_group2s = num_jm_infos;

  _jm_pairs_group2_list = (uint32_t *) 
    malloc (sizeof (uint32_t) * CFG_JM_PAIRS);
  /*
  _jm_pair_subgroups = (jm_pair_subgroup *) 
    malloc (sizeof (jm_pair_subgroup) * _num_jm_pair_subgroups);
  */
  _jm_pair_group2s = (jm_pair_group2 *) 
    malloc (sizeof (jm_pair_group2) * _num_jm_pair_group2s);

  if (!_jm_pairs_group2_list ||
      /*!_jm_pair_subgroups ||*/
      !_jm_pair_group2s)
    {
      fprintf (stderr, "Memory allocation error jm-jm group2s.\n");
      exit(1);
    }

  {
  jm_pair_info_sort prev_pair_info = { { -1, -1, -1, -1 }, -1, 0 };
  jm_pair_group2 *group = _jm_pair_group2s;
  jm_pair_group2 *curgroup = NULL;

  for (i = 0; i < CFG_JM_PAIRS; i++)
    {
      jm_pair_info_sort *jmpisi = &jmpis[i];

      _jm_pairs_group2_list[i] = jmpisi->_pair;

      if (compare_jm_pair_info_sort_jmjm (&prev_pair_info, jmpisi))
	{
	  /* We start a new group. */

	  curgroup = group;

	  curgroup->_info = jmpisi->_info;
	  curgroup->_pairs = &_jm_pairs_group2_list[i];
	  curgroup->_num[0] = curgroup->_num[1];

	  group++;
	}
	
      curgroup->_num[jmpisi->_parity]++;

      prev_pair_info = *jmpisi;
    }

  assert(group - _jm_pair_group2s == (ssize_t) _num_jm_pair_group2s);
  }

  /* Test dump. */
  /*
  for (i = 0; i < _num_jm_pair_group2s; i++)
    {
      jm_pair_group2 *group = &_jm_pair_group2s[i];

      printf ("%3d %3d %3d %3d (%2d %2d)\n",
	      group->_info._j1, group->_info._m1,
	      group->_info._j2, group->_info._m2,
	      group->_num[0], group->_num[1]);
    }
  */
  /* And then the list with major sorting on parity and sum_m */

  qsort (jmpis, CFG_JM_PAIRS, sizeof (jm_pair_info_sort),
	 compare_jm_pair_info_sort_major_summ_parity);

  /* Count number of unique jms. */

  num_jm_infos = 1;

  for (i = 1; i < CFG_JM_PAIRS; i++)
    {
      int ret_jmjm =
	compare_jm_pair_info_sort_major_summ_parity(&jmpis[i-1], &jmpis[i]);

      if (ret_jmjm)
	num_jm_infos++;
    }

  printf ("%zd jm-jm combinations per sum_m (%d .. %d), parity.\n",
	  num_jm_infos, min_sum_m, max_sum_m);

  _num_jm_pair_groups = num_jm_infos;

  _jm_pairs_group_list = (uint32_t *) 
    malloc (sizeof (uint32_t) * CFG_JM_PAIRS);
  _jm_pair_groups = (jm_pair_group *) 
    malloc (sizeof (jm_pair_group) * _num_jm_pair_groups);

  size_t num_summ_parity_jm_pair_groups =
    (size_t) (max_sum_m - min_sum_m) + 2;

  _summ_parity_jm_pair_groups = (jm_pair_group **)
    malloc (sizeof (jm_pair_group *) * (num_summ_parity_jm_pair_groups + 1));
  _summ_parity_jm_pair_groups_min_sum_m = min_sum_m;

  if (!_jm_pairs_group_list ||
      !_jm_pair_groups)
    {
      fprintf (stderr, "Memory allocation error jm-jm groups.\n");
      exit(1);
    }

  {
  jm_pair_info_sort prev_pair_info = { { -1, -1, -1, -1 }, -1, 0 };
  jm_pair_group *group = _jm_pair_groups;
  jm_pair_group *curgroup = NULL;
  int idx = 0;

  _summ_parity_jm_pair_groups[idx] = group;

  for (i = 0; i < CFG_JM_PAIRS; i++)
    {
      jm_pair_info_sort *jmpisi = &jmpis[i];

      _jm_pairs_group_list[i] = jmpisi->_pair;

      if (compare_jm_pair_info_sort_summ_parity(&prev_pair_info, jmpisi))
	{
	  int sum_m = jmpisi->_info._m1 + jmpisi->_info._m2;
	  int parity = jmpisi->_parity;

	  int nidx = sum_m - _summ_parity_jm_pair_groups_min_sum_m + parity;

	  while (idx < nidx)
	    {
	      idx++;
	      _summ_parity_jm_pair_groups[idx] = group;
	    }
	}

      if (compare_jm_pair_info_sort_major_summ_parity (&prev_pair_info, jmpisi))
	{
	  /* We start a new group. */

	  curgroup = group;

	  curgroup->_info = jmpisi->_info;
	  curgroup->_pairs = &_jm_pairs_group_list[i];
	  curgroup->_num = 0;

	  group++;
	}
	
      curgroup->_num++;

      prev_pair_info = *jmpisi;
    }

  assert(group - _jm_pair_groups == (ssize_t) _num_jm_pair_groups);

  assert ((size_t) idx < num_summ_parity_jm_pair_groups);

  while ((size_t) idx < num_summ_parity_jm_pair_groups)
    {
      idx++;
      _summ_parity_jm_pair_groups[idx] = group;
    }
  }

  /* Test dump. */
  /*
  int sum_m;
  int parity;

  for (sum_m = min_sum_m; sum_m <= max_sum_m; sum_m += 2)
    for (parity = 0; parity < 2; parity++)
      {
	printf ("%-3d %d\n", sum_m, parity);

	int idx = sum_m - _summ_parity_jm_pair_groups_min_sum_m + parity;

	jm_pair_group *begin = _summ_parity_jm_pair_groups[idx];
	jm_pair_group *end   = _summ_parity_jm_pair_groups[idx+1];

	jm_pair_group *group;
  
	for (group = begin; group < end; group++)
	  {
	    printf ("%3d %3d %3d %3d (%2d)\n",
		    group->_info._j1, group->_info._m1,
		    group->_info._j2, group->_info._m2,
		    group->_num);
	  }
      }
  */




  /* First find out how many items we really need. */

  size_t a_i, c_i;

  uint64_t num_accum_comb = 0;

  for (a_i = 0; a_i < CFG_JM_PAIRS; a_i++)
    {
      uint32_t anni_pair = _jm_pairs[a_i];

      uint32_t anni_1 = anni_pair & 0x0000ffff;
      uint32_t anni_2 = anni_pair >> 16;

      int anni_m =
	_table_sp_states[anni_1]._m +
	_table_sp_states[anni_2]._m;

      int anni_l =
	_table_sp_states[anni_1]._l +
	_table_sp_states[anni_2]._l;

      for (c_i = 0; c_i < CFG_JM_PAIRS; c_i++)
	{
	  uint32_t crea_pair = _jm_pairs[c_i];

	  uint32_t crea_1 = crea_pair & 0x0000ffff;
	  uint32_t crea_2 = crea_pair >> 16;
	  /*
	  printf ("%zd %zd  %d %d\n", a_i, c_i, crea_1, crea_2);
	  fflush(stdout);
	  */
	  int crea_m =
	    _table_sp_states[crea_1]._m +
	    _table_sp_states[crea_2]._m;

	  int crea_l =
	    _table_sp_states[crea_1]._l +
	    _table_sp_states[crea_2]._l;

	  if (crea_m - anni_m != CFG_2M_FINAL - CFG_2M_INITIAL)
	    continue;

	  if (((anni_l - crea_l) ^ 
	       (CFG_PARITY_FINAL - CFG_PARITY_INITIAL)) & 1)
	    continue;

	  num_accum_comb++;
	  /*
	  {
	    int sp_a = (int) (anni_1 * (2 * CFG_NUM_SP_STATES - anni_1 - 3) / 2 + anni_2-1);
	    int sp_c = (int) (crea_1 * (2 * CFG_NUM_SP_STATES - crea_1 - 3) / 2 + crea_2-1);

	    assert (sp_a >= 0 && sp_a < CFG_TOT_FIRST_SCND);
	    assert (sp_c >= 0 && sp_c < CFG_TOT_FIRST_SCND);

	    int acc_i = sp_a * CFG_TOT_FIRST_SCND + sp_c;

	    _accumulate[acc_i] = 1;
	  }
	  */
	}
    }

  printf ("%" PRIu64 " accumulate combinations.\n", num_accum_comb);

  for (_acc_hash_mask = 1;
       _acc_hash_mask < num_accum_comb * 2; _acc_hash_mask <<= 1)
    ;

  /* printf("%zd\n", _acc_hash_mask); */

  size_t hashed_acc_sz = sizeof (accumulate_hash_item) * _acc_hash_mask;

  _acc_hash_mask -= 1;

  _acc_hash = (accumulate_hash_item *) malloc (hashed_acc_sz);

  if (!_acc_hash)
    {
      fprintf (stderr,
	       "Memory allocation error (%zd bytes).\n", hashed_acc_sz);
      exit(1);
    }

  memset (_acc_hash, 0, hashed_acc_sz);

  /* Fill the table. */

  uint64_t max_coll = 0, sum_coll = 0;

  for (a_i = 0; a_i < CFG_JM_PAIRS; a_i++)
    {
      uint32_t anni_pair = _jm_pairs[a_i];

      uint32_t anni_1 = anni_pair & 0x0000ffff;
      uint32_t anni_2 = anni_pair >> 16;

      int anni_m =
	_table_sp_states[anni_1]._m +
	_table_sp_states[anni_2]._m;

      int anni_l =
	_table_sp_states[anni_1]._l +
	_table_sp_states[anni_2]._l;

      for (c_i = 0; c_i < CFG_JM_PAIRS; c_i++)
	{
	  uint32_t crea_pair = _jm_pairs[c_i];

	  uint32_t crea_1 = crea_pair & 0x0000ffff;
	  uint32_t crea_2 = crea_pair >> 16;
	  /*
	  printf ("%zd %zd  %d %d\n", a_i, c_i, crea_1, crea_2);
	  fflush(stdout);
	  */
	  int crea_m =
	    _table_sp_states[crea_1]._m +
	    _table_sp_states[crea_2]._m;

	  int crea_l =
	    _table_sp_states[crea_1]._l +
	    _table_sp_states[crea_2]._l;

	  if (crea_m - anni_m != CFG_2M_FINAL - CFG_2M_INITIAL)
	    continue;

	  if (((anni_l - crea_l) ^ 
	       (CFG_PARITY_FINAL - CFG_PARITY_INITIAL)) & 1)
	    continue;
	  
	  uint64_t key =
	    (((uint64_t) anni_1) <<  0) |
	    (((uint64_t) anni_2) << 16) |
	    (((uint64_t) crea_1) << 32) |
	    (((uint64_t) crea_2) << 48);

	  uint64_t x = acc_hash_key(key);

	  x ^= x >> 32;

	  uint64_t j = x & _acc_hash_mask;

	  uint64_t coll = 0;

	  while (_acc_hash[j]._key != 0)
	    {
	      j = (j + 1) & _acc_hash_mask;
	      coll++;
	    }

	  if (coll > max_coll)
	    max_coll = coll;
	  sum_coll += coll;

	  _acc_hash[j]._key = key;
	}
    }


  printf ("Hash: %"PRIu64" entries (%.1f), "
          "avg coll = %.2f, max coll = %"PRIu64"\n",
          _acc_hash_mask + 1,
          (double) num_accum_comb / ((double) (_acc_hash_mask + 1)),
          (double) sum_coll / (double) num_accum_comb, max_coll);
}

void accumulate_add(uint64_t key, double value)
{
  uint64_t x = acc_hash_key(key);
  
  x ^= x >> 32;
  
  uint64_t j = x & _acc_hash_mask;
  
  uint64_t coll = 0;
  
  while (_acc_hash[j]._key != key)
    {
      if (_acc_hash[j]._key == 0)
	{
	  fprintf (stderr, "Internal error: accumulate item not found.\n");
	  exit(1);
	}

      j = (j + 1) & _acc_hash_mask;
      coll++;
    }
  
  _acc_hash[j]._value += value;
}

int accumulate_get(uint64_t key, double *value)
{
  uint64_t x = acc_hash_key(key);
  
  x ^= x >> 32;
  
  uint64_t j = x & _acc_hash_mask;
  
  uint64_t coll = 0;
  
  while (_acc_hash[j]._key != key)
    {
      if (_acc_hash[j]._key == 0)
	return 0;

      j = (j + 1) & _acc_hash_mask;
      coll++;
    }
  
  *value = _acc_hash[j]._value;

  return 1;
}



uint32_t    *_nlj_pairs = NULL;
size_t   _num_nlj_pairs = 0;

int compare_uint32_t(const void *p1, const void *p2)
{
  uint32_t v1 = *((const uint32_t *) p1);
  uint32_t v2 = *((const uint32_t *) p2);

  if (v1 < v2)
    return -1;
  return v1 > v2;
}


typedef struct nlj_hash_item_t
{
  uint64_t _key;
  double   _value;
} nlj_hash_item;

uint64_t nlj_hash_key(uint64_t key)
{
  uint64_t x = 0, y = 0;

  y = key;
  x = key;

#define _lcga 6364136223846793005ll
#define _lcgc 1442695040888963407ll

  x = x * _lcga + _lcgc;
  
  x = x ^ (x << 35);
  x = x ^ (x >> 4);
  x = x ^ (x << 17);
  /*
  x = x ^ (x >> 35);
  x = x ^ (x << 4);
  x = x ^ (x >> 17);
  */
  /*
  printf ("%016"PRIx64":%016"PRIx64" -> %016"PRIx64"\n",
          key[0], key[1], x);
  */
  return x ^ y;
}


nlj_hash_item *_nlj_hash;
uint64_t       _nlj_hash_mask = 0;


void prepare_nlj()
{
  /* Each nljm state implies an nlj state.
   * Each jm pair thus implies an nlj state.
   *
   * The possible couplings in j also tell how far it can couple.
   */

  /* We first just enumerate them, and then sort and pick the unique
   * items.  During first enumeration, we may find at most
   * CFG_JM_PAIRS * (CFG_MAX_J+1) configurations.
   */

  size_t max_nlj_pairs = CFG_JM_PAIRS * (CFG_MAX_J+1);

  size_t sz_nlj_pairs = sizeof (uint32_t) * max_nlj_pairs;

  _nlj_pairs = (uint32_t *) malloc (sz_nlj_pairs);

  if (!_nlj_pairs)
    {
      fprintf (stderr, "Memory allocation error (%zd bytes).\n", sz_nlj_pairs);
      exit(1);
    }

  uint32_t *nlj_pair = _nlj_pairs;

  size_t i;

  for (i = 0; i < CFG_JM_PAIRS; i++)
    {
      uint32_t sp_pair = _jm_pairs[i];

      uint32_t sp_1 = sp_pair & 0x0000ffff;
      uint32_t sp_2 = sp_pair >> 16;

      int nlj_1 = _table_sp_states[sp_1]._nlj;
      int nlj_2 = _table_sp_states[sp_2]._nlj;

      int sum_m =
        _table_sp_states[sp_1]._m +
        _table_sp_states[sp_2]._m;

      int parity =
        (_table_sp_states[sp_1]._l +
	 _table_sp_states[sp_2]._l) & 1;

      int j_1 = _table_sp_states[sp_1]._j;
      int j_2 = _table_sp_states[sp_2]._j;

      int min_j = abs(j_1 - j_2);
      int max_j = j_1 + j_2;

      int j12;

      if (min_j < abs(sum_m))
	min_j = abs(sum_m);
      
      for (j12 = min_j; j12 <= max_j; j12 += 2)
	{
	  /*
	  printf ("PAIR: %4d %4d : %3d , %3d ,  %3d , %3d , %d\n",
		  sp_1, sp_2, nlj_1, nlj_2, j12, sum_m, parity);
	  */

	  uint32_t nlj_key =
	    (uint32_t) (nlj_1 | (nlj_2 << 12) | (j12 << 24) | (parity << 31));
	  
	  *(nlj_pair++) = nlj_key;
	}
    }

  size_t num_nlj_pairs = (size_t) (nlj_pair - _nlj_pairs);

  qsort (_nlj_pairs, num_nlj_pairs, sizeof (uint32_t), compare_uint32_t);

  /* Squeeze out the duplicates. */

  uint32_t prev = (uint32_t) -1; /* will not match first item */

  uint32_t *src  = _nlj_pairs;
  uint32_t *dest = _nlj_pairs;

  for (i = 0; i < num_nlj_pairs; i++)
    {
      uint32_t val = *(src++);

      if (val != prev)
	*(dest++) = val;

      prev = val;
    }

  _num_nlj_pairs = (size_t) (dest - _nlj_pairs);  

  printf ("%zd nlj pairs -> %zd\n", num_nlj_pairs, _num_nlj_pairs);

  /* And reallocate. */

  sz_nlj_pairs = sizeof (uint32_t) * _num_nlj_pairs;

  _nlj_pairs = (uint32_t *) realloc (_nlj_pairs, sz_nlj_pairs);

  if (!_nlj_pairs)
    {
      fprintf (stderr,
	       "Memory reallocation error (%zd bytes).\n", sz_nlj_pairs);
      exit(1);
    }

  /* Possible range of jtrans? */

  int jtrans_min = abs(CFG_2J_INITIAL - CFG_2J_FINAL);
  int jtrans_max = CFG_2J_INITIAL + CFG_2J_FINAL;

  int jtrans;

  int mtrans = CFG_2M_INITIAL - CFG_2M_FINAL;

  if (abs(mtrans) > jtrans_max)
    {
      fprintf (stderr, "FIXME: abs(mtrans) > jtrans_max.\n");
      exit(1);
    }

  /* The nlj collection table is then between pairs of (nlj,nlj,J)
   * and (nlj,nlj,J) such that the total coupled J becomes Jtrans.
   */

  /* How many can there be? */

  size_t a_i, c_i;

  size_t num_nlj_comb = 0;

  for (a_i = 0; a_i < _num_nlj_pairs; a_i++)
    {
      uint32_t anni_pair = _nlj_pairs[a_i];
      /*
      uint32_t anni_1 = anni_pair         & 0xfff;
      uint32_t anni_2 = (anni_pair >> 12) & 0xfff;
      */
      int      anni_j = (int) ((anni_pair >> 24) &  0x7f);
      uint32_t anni_parity = anni_pair >> 31;

      for (c_i = 0; c_i < _num_nlj_pairs; c_i++)
	{
	  uint32_t crea_pair = _nlj_pairs[c_i];
	  /*  
	  uint32_t crea_1 = crea_pair         & 0xfff;
	  uint32_t crea_2 = (crea_pair >> 12) & 0xfff;
	  */
	  int      crea_j = (int) ((crea_pair >> 24) &  0x7f);
	  uint32_t crea_parity = crea_pair >> 31;

	  if (((anni_parity ^ crea_parity) ^
	       (CFG_PARITY_FINAL - CFG_PARITY_INITIAL)) & 1)
	    continue;

	  int min_j = abs(anni_j - crea_j);
	  int max_j = anni_j + crea_j;

	  /* Now, can we couple to interesting jtrans? */

	  for (jtrans = min_j; jtrans <= max_j; jtrans += 2)
	    {
	      if (jtrans >= jtrans_min && jtrans <= jtrans_max)
		{
		  num_nlj_comb++;
		}
	    }
	}
    }

  printf ("%zd nlj pair combinations\n", num_nlj_comb);

  for (_nlj_hash_mask = 1;
       _nlj_hash_mask < num_nlj_comb * 2; _nlj_hash_mask <<= 1)
    ;

  /* printf("%zd\n", _nlj_hash_mask); */

  size_t hashed_nlj_sz = sizeof (nlj_hash_item) * _nlj_hash_mask;

  _nlj_hash_mask -= 1;

  _nlj_hash = (nlj_hash_item *) malloc (hashed_nlj_sz);

  if (!_nlj_hash)
    {
      fprintf (stderr,
	       "Memory allocation error (%zd bytes).\n", hashed_nlj_sz);
      exit(1);
    }

  memset (_nlj_hash, -1, hashed_nlj_sz);

  /* Fill the table. */

  uint64_t max_coll = 0, sum_coll = 0;

  for (a_i = 0; a_i < _num_nlj_pairs; a_i++)
    {
      uint32_t anni_pair = _nlj_pairs[a_i];

      uint32_t anni_1 = anni_pair         & 0xfff;
      uint32_t anni_2 = (anni_pair >> 12) & 0xfff;

      int      anni_j = (int) ((anni_pair >> 24) &  0x7f);
      uint32_t anni_parity = anni_pair >> 31;

      for (c_i = 0; c_i < _num_nlj_pairs; c_i++)
	{
	  uint32_t crea_pair = _nlj_pairs[c_i];

	  uint32_t crea_1 = crea_pair         & 0xfff;
	  uint32_t crea_2 = (crea_pair >> 12) & 0xfff;

	  int      crea_j = (int) ((crea_pair >> 24) &  0x7f);
	  uint32_t crea_parity = crea_pair >> 31;

	  if (((anni_parity ^ crea_parity) ^
	       (CFG_PARITY_FINAL - CFG_PARITY_INITIAL)) & 1)
	    continue;

	  int min_j = abs(anni_j - crea_j);
	  int max_j = anni_j + crea_j;

	  /* Now, can we couple to interesting jtrans? */

	  for (jtrans = min_j; jtrans <= max_j; jtrans += 2)
	    {
	      if (jtrans >= jtrans_min && jtrans <= jtrans_max)
		{
		  uint64_t key =
		    (((uint64_t) anni_1) <<  0) |
		    (((uint64_t) anni_2) << 11) |
		    (((uint64_t) crea_1) << 22) |
		    (((uint64_t) crea_2) << 33) |
		    (((uint64_t) anni_j) << 44) |
		    (((uint64_t) crea_j) << 51) |
		    (((uint64_t) jtrans) << 58);
		  
		  uint64_t x = nlj_hash_key(key);

		  x ^= x >> 32;

		  uint64_t j = x & _nlj_hash_mask;

		  uint64_t coll = 0;

		  while (_nlj_hash[j]._key != (uint64_t) -1)
		    {
		      j = (j + 1) & _nlj_hash_mask;
		      coll++;
		    }

		  if (coll > max_coll)
		    max_coll = coll;
		  sum_coll += coll;

		  _nlj_hash[j]._key = key;
		  _nlj_hash[j]._value = 0;
		}
	    }
	}
    }

  printf ("Hash: %"PRIu64" entries (%.1f), "
          "avg coll = %.2f, max coll = %"PRIu64"\n",
          _nlj_hash_mask + 1,
          (double) num_nlj_comb / ((double) (_nlj_hash_mask + 1)),
          (double) sum_coll / (double) num_nlj_comb, max_coll);

}



void nlj_add(uint64_t key, double value)
{
  uint64_t x = nlj_hash_key(key);
  
  x ^= x >> 32;
  
  uint64_t j = x & _nlj_hash_mask;
  
  uint64_t coll = 0;
  
  while (_nlj_hash[j]._key != key)
    {
      if (_nlj_hash[j]._key == (uint64_t) -1)
	{
	  fprintf (stderr, "Internal error: nlj item not found.\n");
	  exit(1);
	}

      j = (j + 1) & _nlj_hash_mask;
      coll++;
    }
  
  _nlj_hash[j]._value += value;
}

int nlj_get(uint64_t key, double *value)
{
  uint64_t x = nlj_hash_key(key);
  
  x ^= x >> 32;
  
  uint64_t j = x & _nlj_hash_mask;
  
  uint64_t coll = 0;
  
  while (_nlj_hash[j]._key != key)
    {
      if (_nlj_hash[j]._key == (uint64_t) -1)
	return 0;

      j = (j + 1) & _nlj_hash_mask;
      coll++;
    }
  
  *value = _nlj_hash[j]._value;

  return 1;
}

void write_nlj()
{
  /* Write the contents of the nlj hash table. */

  /* We will not use it again, so begin by compacting the contents.
   * Still unordered.
   */

  nlj_hash_item *src  = _nlj_hash;
  nlj_hash_item *dest = _nlj_hash;

  size_t i;

  size_t nz = 0;

  for (i = 0; i <= _nlj_hash_mask; i++)
    {
      uint64_t key = src->_key;

      if (key != (uint64_t) -1)
	{
	  if (src->_value)
	    nz++;

	  *(dest++) = *src;
	}

      src++;
    }

  size_t num_nlj_comb = (size_t) (dest - _nlj_hash);

  printf ("%zd nlj pair comb, %zd non-zero.\n", num_nlj_comb, nz);

  int fd = open ("nlj_out.bin",
		 O_WRONLY | O_CREAT | O_TRUNC
#ifdef O_LARGEFILE
		 | O_LARGEFILE
#endif
		 ,
		 S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  
  if (fd == -1)
    {
      perror("open");
      exit(1);
    }

  /* Storing this item here in the file is bad, as it misaligns
   * everything that follows.
   */
  full_write (fd, &num_nlj_comb, sizeof (num_nlj_comb));

  full_write (fd, _nlj_hash, sizeof (nlj_hash_item) * num_nlj_comb);

  close (fd);


  


}
