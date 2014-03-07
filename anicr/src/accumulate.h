#ifndef __ACCUMULATE_H__
#define __ACCUMULATE_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

void prepare_accumulate();

/*****************************************************************************/

typedef struct accumulate_hash_item_t
{
  uint64_t _key;
  double   _value;
} accumulate_hash_item;

extern accumulate_hash_item *_acc_hash;
extern uint64_t              _acc_hash_mask;

inline uint64_t acc_hash_key(uint64_t key)
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

inline void accumulate_pre(uint64_t key, uint64_t *rx)
{
  uint64_t x = acc_hash_key(key);
  
  x ^= x >> 32;
  
  x = x & _acc_hash_mask;

  *rx = x;
}

inline void accumulate_prefetch_r(uint64_t x)
{
  __builtin_prefetch(&_acc_hash[x]._key, 0, 0);
}

inline void accumulate_prefetch_rw(uint64_t x)
{
  __builtin_prefetch(&_acc_hash[x]._key, 1, 0);
}

inline void accumulate_post_add(uint64_t key, uint64_t x, double value)
{
  while (_acc_hash[x]._key != key)
    {
      if (_acc_hash[x]._key == 0)
	{
	  fprintf (stderr, "Internal error: accumulate item not found.\n");
	  exit(1);
	}

      x = (x + 1) & _acc_hash_mask;
    }
  
  _acc_hash[x]._value += value;
}

inline int accumulate_post_get(uint64_t key, uint64_t x, double *value)
{
  while (_acc_hash[x]._key != key)
    {
      if (_acc_hash[x]._key == 0)
	return 0;

      x = (x + 1) & _acc_hash_mask;
    }
  
  *value = _acc_hash[x]._value;

  return 1;
}

/*****************************************************************************/

void prepare_nlj();

void nlj_add(uint64_t key, double value);

int nlj_get(uint64_t key, double *value);

void write_nlj();

/*****************************************************************************/

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

typedef struct jm_pair_group_t
{
  jm_pair_info _info;

  uint32_t *_pairs[2];
  uint32_t  _num[2];

} jm_pair_group;

extern uint32_t *_jm_pairs_group2_list;

extern jm_pair_group *_jm_pair_group2s;
extern size_t     _num_jm_pair_group2s;

extern uint32_t *_jm_pairs_group_list;

extern jm_pair_group  *_jm_pair_groups;
extern size_t      _num_jm_pair_groups;

extern jm_pair_group **_summ_parity_jm_pair_groups;
extern int             _summ_parity_jm_pair_groups_min_sum_m;

/*****************************************************************************/

void alloc_couple_items(size_t max_anni, size_t max_crea);

#endif/*__ACCUMULATE_H__*/
