
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

#include "tmp_config.h"

uint32_t *_jm_pairs = NULL;

void prepare_accumulate()
{
  /* Read in the jm pairs info. */

  size_t sz_jm_pairs = sizeof (uint32_t) * CFG_JM_PAIRS;

  _jm_pairs = (uint32_t*) malloc (sz_jm_pairs);

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


double *_accumulate;

typedef struct accumulate_hash_item_t
{
  uint64_t _key;
  double   _value;
} accumulate_hash_item;

accumulate_hash_item *_acc_hash;
uint64_t              _acc_hash_mask = 0;

void alloc_accumulate()
{
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


  /* The reduced version. */

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
