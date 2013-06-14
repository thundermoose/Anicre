
#ifndef __ANTOINE_STRUCT_HH__
#define __ANTOINE_STRUCT_HH__

#include <stdint.h>

struct mr_antoine_header_new_t
{
  uint32_t num_of_shell;
  uint32_t num_of_jm;
  uint32_t nsd;
  uint32_t A0;
  uint32_t A[2];
  uint32_t nslt[2];
};

struct mr_antoine_header_old_t
{
  uint32_t num_of_shell;
  uint32_t num_of_jm;
  uint32_t nsd;
  uint32_t A[2];
  uint32_t nslt[2];
};

struct mr_antoine_nr_ll_jj_item_t // * num_of_shell
{
  uint32_t nr;
  uint32_t ll;
  uint32_t jj;
};

struct mr_antoine_num_mpr_item_t // * num_of_jm
{
  uint32_t num;
  uint32_t mpr;
};

struct mr_antoine_istate_item_t // * nsd
{
  uint32_t istate[2];
};

struct mr_antoine_occ_item_t // A[i] * nslt[i], i = 1,2
{
  uint32_t sp;
};

struct mr_antoine_fon_ben_t
{
  uint32_t fon[13];
  double   ben;  
};

#endif/*__MAGIC_ANTOINE_READ_HH__*/
