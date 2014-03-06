#ifndef __ACCUMULATE_H__
#define __ACCUMULATE_H__

#include <stdint.h>
#include <stdlib.h>

void prepare_accumulate();

void accumulate_add(uint64_t key, double value);

int accumulate_get(uint64_t key, double *value);

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

#endif/*__ACCUMULATE_H__*/
