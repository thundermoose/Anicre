#ifndef __MP_STATES_H__
#define __MP_STATES_H__

#include "anicr_config.h"

#include <stdint.h>

extern uint64_t *_hashed_mp;

extern uint64_t  _hash_mask;
extern uint32_t  _hash_stride;
extern int       _hash_stride_shift;

extern uint64_t _lookups;
extern uint64_t _found;

inline uint64_t packed_hash_key(uint64_t *key)
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



inline int find_mp_state(uint64_t *lookfor, double *val)
{
  _lookups++;

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

	{
	  _found++;
	  *val = *(((double *) p) + CFG_PACK_WORDS);
	  
	  return 1;
	}
	
      not_found:
	if (!*p)
	  {
	    return 0;
	  }
	
	j = (j + _hash_stride) & _hash_mask;
      }
  }
}


#endif/*__MP_STATES_H__*/
