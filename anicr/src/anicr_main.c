
#include "anicr_config.h"
#include "create.h"
#include "packed_create.h"

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

uint64_t packed_hash_key(uint64_t *key)
{
  uint64_t x = 0, y = 0;
  int i;

  for (i = 0; i < CFG_PACK_WORDS; i++)
    {
      y ^= key[i];
      x ^= key[i];

#define _lcga 6364136223846793005ll
#define _lcgc 1442695040888963407ll

      y = y * _lcga + _lcgc;

      x = x ^ (x << 35);
      x = x ^ (x >> 4);
      x = x ^ (x << 17);
    }

  x = x ^ (x >> 35);
  x = x ^ (x << 4);
  x = x ^ (x >> 17);
  /*
  printf ("%016"PRIx64":%016"PRIx64" -> %016"PRIx64"\n",
	  key[0], key[1], x);
  */
  return x ^ y;
}

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

  printf ("%"PRIu64" %zd\n",_hash_mask, hashed_mp_sz);

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

  size_t toread = mp_sz;
  void *dest = _mp;

  while (toread)
    {
      ssize_t n = read (fd, dest, toread);

      if (n == -1)
	{
	  perror("read");
	  exit(1);
	}
      if (n == 0)
	{
	  fprintf (stderr, "End-of-file reading %zd, missing %zd.\n",
		   mp_sz, toread);
	  exit(1);
	}
      toread -= (size_t) n;
      dest += n;
    }

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

  mp = _mp;

  int packed = 0;

  if (argc > 1 && strcmp(argv[1],"--packed") == 0)
    packed = 1;

  for (i = 0; i < num_mp; i++)
    {
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

  return 0;
}

int find_mp_state(uint64_t *lookfor)
{
  /* size_t num_sp = CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1; */

#if 0
  size_t num_mp = CFG_NUM_MP_STATES;

  void *found =
    bsearch (lookfor,
	     _mp, num_mp, sizeof (uint64_t) * (CFG_PACK_WORDS + CFG_WAVEFCNS),
	     compare_packed_mp_state);
#endif

  void *found;

  {
    uint64_t x = packed_hash_key(lookfor);
    
    x ^= x >> 32;
    
    uint64_t j = (x << _hash_stride_shift) & _hash_mask;

    for ( ; ; )
      {
	uint64_t *p = &_hashed_mp[j];
	
	int k;
	
	for (k = 0; k < CFG_PACK_WORDS; k++)
	  {
	    if (p[k] != lookfor[k])
	      goto not_found;
	  }
	
	found = p;
	break;
	
      not_found:
	if (!*p)
	  {
	    found = NULL;
	    break;
	  }
	
	j = (j + _hash_stride) & _hash_mask;
      }
  }


  if (found)
    _found++;
  _lookups++;

  return found != NULL;
}
