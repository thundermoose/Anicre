
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
  int32_t  mpr;
};

struct mr_antoine_istate_item_t // * nsd
{
  uint32_t istate[2];
};

struct mr_antoine_occ_item_t // A[i] * nslt[i], i = 1,2
{
  uint32_t sp;
};

union mr_antoine_fon_new_t
{
  uint32_t raw[14];
  struct
  {
    uint32_t dummy0[6];
    uint32_t mjtotal; /* (7) */
    uint32_t iparity; /* (8) */
    uint32_t dummy9;
    uint32_t jt2;   /* (10) */ /* Jx = jt2*0.5 */
    uint32_t dummy11;
    uint32_t coul;  /* (12) */
    uint32_t dummy13;
    uint32_t iprec; /* (14) */
  } _;
};

union mr_antoine_fon_old_t
{
  uint32_t raw[13];
  struct
  {
    uint32_t dummy0[5];
    uint32_t mjtotal; /* (6) */
    uint32_t iparity; /* (7) */
    uint32_t dummy8;
    uint32_t jt2;   /* (9) */ /* Jx = jt2*0.5 */
    uint32_t dummy10;
    uint32_t coul;  /* (11) */
    uint32_t dummy12;
    /* not clear if iprec is in old */
    uint32_t iprec; /* (13) */
  } _;
};

#endif/*__ANTOINE_STRUCT_HH__*/
