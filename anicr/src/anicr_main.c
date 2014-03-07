
#include "anicr_config.h"
#include "create.h"
#include "packed_create.h"
#include "accumulate.h"
#include "util.h"
#include "mp_states.h"

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

uint64_t *_mp = NULL;

uint64_t *_hashed_mp = NULL;

uint64_t  _hash_mask = 0;
uint32_t  _hash_stride = 0;
int       _hash_stride_shift = 0;

uint64_t _lookups = 0;
uint64_t _found = 0;

double   _cur_val;

int main(int argc, char *argv[])
{
  size_t num_mp = CFG_NUM_MP_STATES;
  /* size_t num_sp = CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1; */

  assert(sizeof (uint64_t) == sizeof (double));

  size_t mp_sz = sizeof (uint64_t) * (CFG_PACK_WORDS + CFG_WAVEFCNS) * num_mp;

  _mp = (uint64_t *) malloc (mp_sz);

  if (!_mp)
    {
      fprintf (stderr, "Memory allocation error (%zd bytes).\n", mp_sz);
      exit(1);
    }

  for (_hash_mask = 1; _hash_mask < num_mp * 2; _hash_mask <<= 1)
    ;

  for (_hash_stride_shift = 0;
       (1 << _hash_stride_shift) < CFG_PACK_WORDS + CFG_WAVEFCNS; 
	_hash_stride_shift++)
    ;

  _hash_stride = ((uint32_t) 1) << _hash_stride_shift;
  _hash_mask *= _hash_stride;
  
  size_t hashed_mp_sz = sizeof (uint64_t) * _hash_mask;

  _hash_mask -= 1;

  /* printf ("%"PRIu64" %zd\n",_hash_mask, hashed_mp_sz); */

  _hashed_mp = (uint64_t *) malloc (hashed_mp_sz);

  if (!_hashed_mp)
    {
      fprintf (stderr, "Memory allocation error (%zd bytes).\n", hashed_mp_sz);
      exit(1);
    }

  memset (_hashed_mp, 0, hashed_mp_sz);

  int fd = open ("states_all_orig.bin", O_RDONLY);

  if (fd == -1)
    {
      perror("open");
      exit(1);
    }

  full_read (fd, _mp, mp_sz);

  close (fd);

  printf ("Read %zd mp states.\n", num_mp);

  size_t i;

  uint64_t *mp = _mp;

  uint64_t max_coll = 0, sum_coll = 0;

  for (i = 0; i < num_mp; i++)
    {
      uint64_t x = packed_hash_key(mp);

      x ^= x >> 32;

      uint64_t j = (x << _hash_stride_shift) & _hash_mask;

      uint64_t coll = 0;

      while (_hashed_mp[j] != 0)
	{
	  j = (j + _hash_stride) & _hash_mask;
	  coll++;
	}

      if (coll > max_coll)
	max_coll = coll;
      sum_coll += coll;
      
      uint32_t k;

      for (k = 0; k < CFG_PACK_WORDS; k++)
	{
	  _hashed_mp[j + k] = mp[k];
	}
      for (k = 0; k < CFG_WAVEFCNS; k++)
	{
	  _hashed_mp[j + CFG_PACK_WORDS + k] =
	    mp[CFG_PACK_WORDS + k];
	}

      mp += (CFG_PACK_WORDS + CFG_WAVEFCNS);
    }

  printf ("Hash: %"PRIu64" entries (%.1f), "
	  "avg coll = %.2f, max coll = %"PRIu64"\n",
	  _hash_mask + 1,
	  (double) num_mp / ((double) (_hash_mask + 1) / _hash_stride),
	  (double) sum_coll / (double) num_mp, max_coll);
#if 0 
  /* It turns out that lookup is ~ 20 % faster with original states
   * in original antoine order...
   */
 
  qsort (_mp, num_mp, sizeof (uint64_t) * (CFG_PACK_WORDS + CFG_WAVEFCNS),
	 compare_packed_mp_state);
  
  printf ("Sorted %zd mp states.\n", num_mp);
#endif

  ammend_tables();

  prepare_accumulate();

  prepare_nlj();

  alloc_accumulate();

  mp = _mp;

  int packed = 0;

  if (argc > 1 && strcmp(argv[1],"--packed") == 0)
    packed = 1;

  for (i = 0; i < num_mp; i++)
    {
      _cur_val = *(((double *) mp) + CFG_PACK_WORDS);

      if (packed)
	{
	  packed_annihilate_states(mp);
	}
      else
	{
	  /* annihilate_states(mp + CFG_NUM_SP_STATES0, mp); */
	  annihilate_packed_states(mp);
	}

      mp += (CFG_PACK_WORDS + CFG_WAVEFCNS);
      
      if (i % 10000 == 0)
	{
	  printf ("anicr %zd / %zd\r", i, num_mp);
	  fflush (stdout);
	}
    }

  printf ("Annihilated-created for %zd mp states.\n", num_mp);

  printf ("Found %"PRIu64"/%"PRIu64".\n", _found, _lookups);
  
  /* couple_accumulate(); */

  couple_accumulate_2();

  write_nlj();
  
  return 0;
}

