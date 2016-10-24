#ifndef __MP_STATES_H__
#define __MP_STATES_H__

#include "anicr_config.h"

#include <stdint.h>

typedef struct hash_mp_wf_item_t
{
  uint64_t _mp[CFG_PACK_WORDS];
#if CFG_CONN_TABLES || CFG_IND_TABLES
  uint64_t _index;
#else
  double   _wf[CFG_WAVEFCNS];
#endif
#if CFG_HASH_MP_PAD64 != 0
  uint64_t _dummy[CFG_HASH_MP_PAD64];
#endif
} hash_mp_wf_item;

typedef struct hash_mp_wf_t
{
  hash_mp_wf_item *_hashed;
  uint64_t         _hash_mask;
} hash_mp_wf;

extern hash_mp_wf *_hashed_mp;

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

inline void find_mp_state_pre(uint64_t *lookfor, uint64_t *rx)
{
  uint64_t x = packed_hash_key(lookfor);
  
  x ^= x >> 32;
  
  x = x & _hashed_mp->_hash_mask;

  *rx = x;
}

inline void find_mp_state_prefetch(uint64_t x)
{
  hash_mp_wf_item *p = &_hashed_mp->_hashed[x];

  __builtin_prefetch(p, 0, 0);
}

inline int find_mp_state_post(uint64_t *lookfor, uint64_t x, 
#if CFG_CONN_TABLES || CFG_IND_TABLES
			      uint64_t *ind
#else
			      double *val
#endif
			      )
{
  _lookups++;

  for ( ; ; )
    {
      hash_mp_wf_item *p = &_hashed_mp->_hashed[x];
      
      int k;
      
      for (k = 0; k < CFG_PACK_WORDS; k++)
	{
	  if (p->_mp[k] != lookfor[k])
	    goto not_found;
	}
      
      {
	_found++;
#if CFG_CONN_TABLES || CFG_IND_TABLES
	*ind = p->_index;
#else
	*val = p->_wf[0];
#endif
	
	return 1;
      }
      
    not_found:
      if (!p->_mp[0])
	{
	  return 0;
	}
      
      x = (x + 1) & _hashed_mp->_hash_mask;
    }
}


#endif/*__MP_STATES_H__*/
