
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "anicr_tables.h"
#include "anicr_config.h"

#include "create.h"
#include "accumulate.h"
#include "mp_states.h"

#include "code.h"

#include "tmp_config.h"

/* Annihilate states. */

#define NSP  CFG_NUM_SP_STATES0

#define SP_STATE_E(sp) (2*(sp)._n+(sp)._l)

#define sp_info _table_sp_states

uint32_t sp_jmEp[CFG_NUM_SP_STATES];

#define SHIFT_J  28
#define SHIFT_M  23
#define SHIFT_E  18
#define SHIFT_P  17

#define EXTRACT_SP(x)      ((x) & 0x1ffff)
#define GET_P(x)           ((x) >> SHIFT_P)
#define EXTRACT_E(x)       (((x) >> SHIFT_E) & 0x1f)
#define EXTRACT_CMPR_M(x)  (((x) >> SHIFT_M) & 0x1f)
#define EXTRACT_JM(x)      (((x) >> SHIFT_M) & 0x1ff)

void ammend_table(uint32_t *table, int nmemb)
{
  int i;

  for (i = 0; i < nmemb; i++)
    {
      uint32_t sp = table[i];
      
      int n = sp_info[sp]._n;
      int l = sp_info[sp]._l;
      int j = sp_info[sp]._j;
      int m = sp_info[sp]._m;

      int cmpr_j = (j - 1) / 2; /* = j / 2, j always odd here */
      int cmpr_m = (m + j) / 2;
      int E = 2 * n + l;

      table[i] = sp |
	(uint32_t) ((cmpr_j << SHIFT_J) |
		    (cmpr_m << SHIFT_M) |
		    (E      << SHIFT_J) |
		    ((l & 1) << SHIFT_P));
    }
}

void ammend_tables()
{
  int i;

  /* Such that it can be filled by ammend_table :-) */

  for (i = 0; i < CFG_NUM_SP_STATES; i++)
    sp_jmEp[i] = (uint32_t) i;

  ammend_table(sp_jmEp, CFG_NUM_SP_STATES);    

  ammend_table(_table_1_0_info._miss,
	       _table_1_0_info._offset[2 * _table_1_0_info._parity_stride]);
  ammend_table(_table_2_0_info._miss,
	       _table_2_0_info._offset[2 * _table_2_0_info._parity_stride]);
  ammend_table(_table_3_0_info._miss,
	       _table_3_0_info._offset[2 * _table_3_0_info._parity_stride]);

}


void create_states(int *in_sp_other,
		   int *in_sp,
#if ANICR2
		   int sp_anni1, int sp_anni2, int sp_crea1,
		   int fill,
#else
		   int sp_anni,
#endif
		   int phase_i,
		   int miss_parity, int miss_m, int E);

void create_states_1st(int *in_sp_other,
		       int *in_sp, int sp_anni1, int sp_anni2,
		       int phase_i,
		       int miss_parity, int miss_m, int E);

void create_states_2nd(int *in_sp_other,
		       int *in_sp, int sp_anni1, int sp_anni2,
		       int sp_crea1,
		       int miss_parity, int miss_m, int E);

void created_state(int *in_sp_other,
		   int *in_sp,
#if ANICR2
		   int sp_anni1, int sp_anni2,
		   int sp_crea1, int sp_crea2,
#else
		   int sp_anni, int sp_crea,
#endif
		   int phase_i
		   );

void annihilate_states_2nd(int *in_sp_other,
			   int *in_sp,
			   int sp_anni1,
			   int phase_i,
			   int miss_parity, int miss_m, int E);

void annihilate_packed_states(uint64_t *packed)
{
  int list[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

  packed_to_int_list(list,packed);

  annihilate_states(list + CFG_NUM_SP_STATES0, list);
}

void annihilate_states(int *in_sp_other,
		       int *in_sp)
{
  int i;

  /* Print the state. */

#if DEBUG_ANICR
  printf ("==============================================================================\n");
  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]);
  printf (" :");
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]);
  printf ("\n");
#endif

  /* Delete 1 state. */

  int out_sp[NSP];

  int E = 0;
  /* int M = 0; */

  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    {
      /* M += sp_info[in_sp_other[i]]._m; */
      E += SP_STATE_E(sp_info[in_sp_other[i]]);
#if DEBUG_ANICR
      printf ("%3d  %3d %3d\n",
	      E,
	      sp_info[in_sp_other[i]]._m, SP_STATE_E(sp_info[in_sp_other[i]]));
#endif
    }

  for (i = 1; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
      /* M += sp_info[in_sp[i]]._m; */
      E += SP_STATE_E(sp_info[in_sp[i]]);
#if DEBUG_ANICR
      printf ("%3d  %3d %3d\n",
	      E,
	      sp_info[in_sp[i]]._m, SP_STATE_E(sp_info[in_sp[i]]));
#endif
    }

#if DEBUG_ANICR
  printf ("E=%3d\n", E);
#endif

  /* The out_sp list is missing sp state 0. */

#if ANICR2
  annihilate_states_2nd(in_sp_other,
			out_sp, in_sp[0], 0,
			sp_info[in_sp[0]]._l,
			sp_info[in_sp[0]]._m, E);
#else
  create_states(in_sp_other,
		out_sp,
		in_sp[0], 0,
		sp_info[in_sp[0]]._l & 1,
		sp_info[in_sp[0]]._m, E);
#endif

  E += SP_STATE_E(sp_info[in_sp[0]]);

  /* And now try with all other missing ones. */

  for (i = 0; i < NSP - (ANICR2 ? 2 : 1); i++)
    {
      /* We always have the space at [0] empty. */

      out_sp[i+1] = in_sp[i];

#if ANICR2
      annihilate_states_2nd(in_sp_other,
			    out_sp, in_sp[i+1], i+1,
			    sp_info[in_sp[i+1]]._l,
			    sp_info[in_sp[i+1]]._m,
			    E - SP_STATE_E(sp_info[in_sp[i+1]]));
#else
      create_states(in_sp_other,
		    out_sp, 
		    in_sp[i+1], i+1,
		    sp_info[in_sp[i+1]]._l & 1,
		    sp_info[in_sp[i+1]]._m,
		    E - SP_STATE_E(sp_info[in_sp[i+1]]));
#endif
    }
}

void annihilate_states_2nd(int *in_sp_other,
			   int *in_sp, int sp_anni1,
			   int phase_i,
			   int miss_parity, int miss_m, int E)
{
  int i;

  /* Print the state. */

#if DEBUG_ANICR
  printf ("===------------------------------------------------------------------------===\n");
  for (i = 1; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]);
  printf (" :");
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]);
  printf ("\n");
#endif

#if DEBUG_ANICR
  printf (" : E=%3d  ~m=%3d  ~p=%d\n", E, miss_m, miss_parity);
#endif

  /* Delete 1 state. */

  int out_sp[NSP];

  for (i = 2; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }

  /* The out_sp list is missing sp state 0 and 1. */

  if (sp_anni1 < in_sp[1])
    create_states_1st(in_sp_other,
		      out_sp, sp_anni1, in_sp[1], phase_i ^ 1,
		      (sp_info[in_sp[1]]._l ^ miss_parity) & 1,
		      miss_m + sp_info[in_sp[1]]._m,
		      E - SP_STATE_E(sp_info[in_sp[1]]));

  /* And now try with all other missing ones. */

  for (i = 1; i < NSP - 1; i++)
    {
      /* We always have the space at [0] and [1] empty. */

      out_sp[i+1] = in_sp[i];

      if (sp_anni1 < in_sp[i+1])
	create_states_1st(in_sp_other,
			  out_sp, sp_anni1, in_sp[i+1], phase_i ^ (i+1),
			  (sp_info[in_sp[i+1]]._l ^ miss_parity) & 1,
			  miss_m + sp_info[in_sp[i+1]]._m,
			  E - SP_STATE_E(sp_info[in_sp[i+1]]));
    }
}

/* Create states, given that we have the list of states that survived
 * the annihilation.
 */

void create_states(int *in_sp_other,
		   int *in_sp,
#if ANICR2
                   int sp_anni1, int sp_anni2, int sp_crea1,
		   int fill,
#else
                   int sp_anni,
#endif
		   int phase_i,
		   int miss_parity, int miss_m, int E)
{
  int i;

  int out_sp[NSP + 1];

  /* We are missing a certain m, and also have a known
   * energy.
   */

  info_state_for_miss *miss_info = &_table_1_0_info;

  /* Print the state. */

#if DEBUG_ANICR
#if ANICR2
  printf ("--- a %3d a %3d c %3d --------------------------------------------------------\n", sp_anni1, sp_anni2, sp_crea1);
#else
  printf ("--- a %3d --------------------------------------------------------------------\n", sp_anni);
#endif
#endif

#if DEBUG_ANICR
#if ANICR2
  for (i = 0; i < fill; i++)
    printf (" %4d", in_sp[i]); 
  for (i = fill + 1; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]); 
#else
  for (i = ANICR2 ? 2 : 1; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]); 
#endif
  printf (" : E=%3d  ~m=%3d  ~p=%d\n", E, miss_m, miss_parity);
#endif

  /* Find the list of potential sp states to use. */

  assert (miss_m >= miss_info->_m_min &&
	  miss_m <  miss_info->_m_min + miss_info->_m_steps);

  int max_add_E = CFG_MAX_SUM_E - E;

  if (max_add_E >= miss_info->_num_E)
    max_add_E = miss_info->_num_E - 1;

  int table_end_E = (max_add_E + miss_parity) / 2 + 1;

  int offset_poss_sp =
    miss_info->_offset[miss_info->_parity_stride * miss_parity +
		       miss_info->_m_stride *
		       (miss_m - miss_info->_m_min) / 2];
  int offset_poss_sp_end =
    miss_info->_offset[miss_info->_parity_stride * miss_parity +
		       miss_info->_m_stride *
		       (miss_m - miss_info->_m_min) / 2 +
		       table_end_E];

#if DEBUG_ANICR
  printf ("max_add_E=%3d [%d] -> %d states (%d)\n",
	  max_add_E, table_end_E,
	  offset_poss_sp_end - offset_poss_sp,
	  offset_poss_sp);
#endif

  state_for_miss_m_N *poss_sp_ptr/*, *poss_sp_end*/;

  poss_sp_ptr = &miss_info->_miss[offset_poss_sp];
  /* poss_sp_end = &miss_info->_miss[offset_poss_sp_end]; */

  int num_poss_sp = offset_poss_sp_end - offset_poss_sp;

  /* Now work through the list. 
   *
   * The list is sorted such that the states come in order, and
   * so is the in_sp list.  I.e. we are performing a merge sort.
   */

  if (!num_poss_sp)
    return;

  /* Assume we will begin by inserting a lowest state. */

#if ANICR2
  for (i = 0; i < fill; i++)
    {
      out_sp[i] = in_sp[i];
    }
  for (i = fill+1; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }
#else
  for (i = 1; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }
#endif

  /* Make sure that we do move fill beyond the end. */
  out_sp[NSP] = INT_MAX;

#if ANICR2
  /* Skip past states which are smaller than the one already added. */
  
  for ( ; num_poss_sp; --num_poss_sp, poss_sp_ptr++)
    {
      uint32_t poss_sp = *poss_sp_ptr;
      uint32_t crea_sp = EXTRACT_SP(poss_sp);

      if (crea_sp > (uint32_t) sp_crea1)
	break;
    }
#else
  int fill = 0;
#endif

  for ( ; num_poss_sp; --num_poss_sp, poss_sp_ptr++)
    {
      uint32_t poss_sp = *poss_sp_ptr;
      uint32_t crea_sp = EXTRACT_SP(poss_sp);

      while (crea_sp > (uint32_t) out_sp[fill+1])
	{
	  out_sp[fill] = out_sp[fill+1];
	  fill++;
	}

      if (crea_sp == (uint32_t) out_sp[fill+1])
	{
#if DEBUG_ANICR
	  printf ("%4d x %3d *\n", crea_sp, fill);
#endif
	  continue;
	}

      out_sp[fill] = (int) crea_sp;

#if DEBUG_ANICR
      printf ("%4d @ %3d\n", crea_sp, fill);
#endif

      created_state(in_sp_other,
		    out_sp,
#if ANICR2
		    sp_anni1, sp_anni2, sp_crea1,
#else
		    sp_anni,
#endif
		    (int) crea_sp, phase_i ^ fill);
    }


}

void create_states_1st(int *in_sp_other,
		       int *in_sp, int sp_anni1, int sp_anni2,
		       int phase_i,
		       int miss_parity, int miss_m, int E)
{
  int i;

  int out_sp[NSP + 1];

  (void) phase_i;

  /* We are missing a certain m, and also have a known
   * energy.
   */

  info_state_for_miss *miss_info = &_table_2_0_info;

  /* Print the state. */

#if DEBUG_ANICR
#if ANICR2
  printf ("--- a %3d a %3d --------------------------------------------------------------\n", sp_anni1, sp_anni2);
#else
  printf ("----a %3d --------------------------------------------------------------------\n", sp_anni1);
#endif
#endif

#if DEBUG_ANICR
  for (i = 2; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]); 
  printf (" : E=%3d  ~m=%3d  ~p=%d\n", E, miss_m, miss_parity);
#endif

  /* Find the list of potential sp states to use. */

  assert (miss_m >= miss_info->_m_min &&
	  miss_m <  miss_info->_m_min + miss_info->_m_steps);

  int max_add_E = CFG_MAX_SUM_E - E;

  if (max_add_E >= miss_info->_num_E)
    max_add_E = miss_info->_num_E - 1;

  int table_end_E = ((max_add_E + miss_parity) & ~1) + 2;

#if DEBUG_ANICR
  printf ("%d %d %d %d %d\n",
	  miss_info->_parity_stride, miss_parity,
	  miss_info->_num_E, miss_m, miss_info->_m_min);
#endif

  int offset_poss_sp =
    miss_info->_offset[miss_info->_parity_stride * miss_parity +
		       miss_info->_m_stride *
		       (miss_m - miss_info->_m_min) / 2];
  int cont_info =
    miss_info->_offset[miss_info->_parity_stride * miss_parity +
		       miss_info->_m_stride *
		       (miss_m - miss_info->_m_min) / 2 +
		       table_end_E - 1];
  int offset_poss_sp_end =
    miss_info->_offset[miss_info->_parity_stride * miss_parity +
		       miss_info->_m_stride *
		       (miss_m - miss_info->_m_min) / 2 +
		       table_end_E];

#if DEBUG_ANICR
  printf ("max_add_E=%3d -> %d states (%d)\n",
	  max_add_E, offset_poss_sp_end - offset_poss_sp,
	  offset_poss_sp);
#endif

  state_for_miss_m_N *poss_sp_ptr/*, *poss_sp_end*/;

  poss_sp_ptr = &miss_info->_miss[offset_poss_sp];
  /* poss_sp_end = &miss_info->_miss[offset_poss_sp_end]; */

  int num_poss_sp = offset_poss_sp_end - offset_poss_sp;

  /* Now work through the list. 
   *
   * The list is sorted such that the states come in order, and
   * so is the in_sp list.  I.e. we are performing a merge sort.
   */

  if (!num_poss_sp)
    return;

  /* Assume we will begin by inserting a lowest state. */

  for (i = 2; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }

  /* Make sure that we do move fill beyond the end. */
  out_sp[NSP] = INT_MAX;

  int fill = 0;

  for ( ; ; )
    {
      for ( ; num_poss_sp; --num_poss_sp, poss_sp_ptr++)
	{
	  uint32_t poss_sp = *poss_sp_ptr;
	  uint32_t crea_sp = EXTRACT_SP(poss_sp);

	  while (crea_sp > (uint32_t) out_sp[fill+2])
	    {
	      out_sp[fill] = out_sp[fill+2];
	      fill++;
	    }

#if DEBUG_ANICR
	  printf ("===---===\n");
#endif

	  if (crea_sp == (uint32_t) out_sp[fill+2])
	    {
#if DEBUG_ANICR
	      printf ("%4d x %3d *\n", crea_sp, fill);
#endif
	      continue;
	    }

	  out_sp[fill] = (int) crea_sp;

#if DEBUG_ANICR
	  printf ("%4d @ %3d\n", crea_sp, fill);
#endif

#if ANICR2
	  create_states(in_sp_other,
			out_sp, sp_anni1, sp_anni2,
			(int) crea_sp, fill+1,  phase_i ^ fill,
			(sp_info[crea_sp]._l ^ miss_parity) & 1,
			miss_m - sp_info[crea_sp]._m,
			E + SP_STATE_E(sp_info[crea_sp]));
#else
	  (void) in_sp_other;
	  (void) sp_anni1;
	  (void) sp_anni2;
	  (void) crea_sp;
#endif
	}
      if (!cont_info)
	break;

      num_poss_sp = cont_info >> 18;
      poss_sp_ptr += cont_info & ((1 << 18) - 1);
      cont_info = 0;
    }


}

extern double *_accumulate;

extern double _cur_val;

void created_state(int *in_sp_other,
		   int *in_sp,
#if ANICR2
		   int sp_anni1, int sp_anni2,
		   int sp_crea1, int sp_crea2,
#else
		   int sp_anni, int sp_crea,
#endif
		   int phase_i
		   )
{
  int i;

  /* We need to find the created state in the destination hash table.
   * To get its coefficient.
   */

  (void) in_sp_other;
  (void) in_sp;
#if ANICR2
  (void) sp_anni1;
  (void) sp_anni2;
  (void) sp_crea1;
  (void) sp_crea2;

  /* printf ("%d %d %d %d\n", sp_anni1, sp_anni2, sp_crea1, sp_crea2); */
#else
  (void) sp_anni;
  (void) sp_crea;
#endif

#if 0
  uint32_t jm_a1 = EXTRACT_JM(sp_jmEp[sp_anni1]);
  uint32_t jm_a2 = EXTRACT_JM(sp_jmEp[sp_anni2]);
  uint32_t jm_c1 = EXTRACT_JM(sp_jmEp[sp_crea1]);
  uint32_t jm_c2 = EXTRACT_JM(sp_jmEp[sp_crea2]);
  /*
  printf ("%3d %3d %3d %3d   %3x %3x %3x %3x\n",
	  sp_anni1, sp_anni2, sp_crea1, sp_crea2,
	  jm_a1, jm_a2, jm_c1, jm_c2);
  */

  uint64_t jms =
    jm_a1 | (((uint64_t) jm_a2) << 9) |
    (((uint64_t) jm_c1) << 18) | (((uint64_t) jm_c2) << 27);

  (void) jms;
#endif

#if ANICR2
#if ACC_TABLE
  int sp_a = sp_anni1 * (2 * CFG_NUM_SP_STATES - sp_anni1 - 3) / 2 + sp_anni2-1;
  int sp_c = sp_crea1 * (2 * CFG_NUM_SP_STATES - sp_crea1 - 3) / 2 + sp_crea2-1;
  /*
  printf ("%3d %3d %3d %3d   %6d %6d\n",
	  sp_anni1, sp_anni2, sp_crea1, sp_crea2,
	  sp_a, sp_c);
  */

  assert (sp_a >= 0 && sp_a < CFG_TOT_FIRST_SCND);
  assert (sp_c >= 0 && sp_c < CFG_TOT_FIRST_SCND);

  int acc_i = sp_a * CFG_TOT_FIRST_SCND + sp_c;

  (void) acc_i;
#endif
#else
  int acc_i = sp_anni * CFG_NUM_SP_STATES + sp_crea;
#endif

  uint64_t key =
    (((uint64_t) sp_anni1) <<  0) |
    (((uint64_t) sp_anni2) << 16) |
    (((uint64_t) sp_crea1) << 32) |
    (((uint64_t) sp_crea2) << 48);

  uint64_t acc_x;

  accumulate_pre(key, &acc_x);
  accumulate_prefetch_rw(acc_x);

  /*
  int lookfor[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    lookfor[i] = in_sp[i];
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    lookfor[CFG_NUM_SP_STATES0 + i] = in_sp_other[i];
  */

  (void) i;

  uint64_t lookfor_packed[CFG_PACK_WORDS];

  int_list2_to_packed(lookfor_packed, in_sp, in_sp_other);

  /*
#if DEBUG_ANICR
  for (i = 0; i < CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1; i++)
    printf (" %4d", lookfor[i]);
  printf ("\n");
#endif
  */

  uint64_t lookfor_x;

  find_mp_state_pre(lookfor_packed, &lookfor_x);
  find_mp_state_prefetch(lookfor_x);

  double val;

  if (!find_mp_state_post(lookfor_packed, lookfor_x, &val))
    {
      fprintf (stderr, "NOT FOUND!\n");
      exit (1);
    }
  /* printf ("%4d %4d\n", sp_anni, sp_crea); */

  int sign = 1 - 2 * (phase_i & 1);

#if ACC_TABLE
  _accumulate[acc_i] += val * _cur_val * sign;
#endif

  accumulate_advance_add(key, &acc_x);
  accumulate_post_add(acc_x, val * _cur_val * sign);

#if DEBUG_ANICR
  printf ("%5d %15.10f\n", acc_i, val * _cur_val * sign);
#endif
}

