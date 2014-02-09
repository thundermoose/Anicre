
#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "anicr_tables.h"
#include "anicr_config.h"

#include "create.h"

#define DEBUG_ANICR 0

#define ANICR2      1

/* Annihilate states. */

#define NSP  CFG_NUM_SP_STATES0

#define SP_STATE_E(sp) (2*(sp)._n+(sp)._l)

#define sp_info _table_sp_states

#define SHIFT_J  27
#define SHIFT_M  22
#define SHIFT_E  17
#define SHIFT_P  16

#define EXTRACT_SP(x)      ((x) & 0xffff)
#define GET_P(x)           ((x) >> SHIFT_P)
#define EXTRACT_E(x)       (((x) >> SHIFT_E) & 0x1f)
#define EXTRACT_CMPR_M(x)  (((x) >> SHIFT_M) & 0x1f)
#define EXTRACT_JM(x)      (((x) >> SHIFT_M) & 0x3ff)

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

      int cmpr_m = (m - j) / 2;
      int E = 2 * n + l;

      table[i] = sp |
	(uint32_t) ((j      << SHIFT_J) |
		    (cmpr_m << SHIFT_M) |
		    (E      << SHIFT_J) |
		    ((l & 1) << SHIFT_P));
    }
}

void ammend_tables()
{
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
		   int miss_parity, int miss_m, int E);

void create_states_1st(int *in_sp_other,
		       int *in_sp, int sp_anni1, int sp_anni2,
		       int miss_parity, int miss_m, int E);

void create_states_2nd(int *in_sp_other,
		       int *in_sp, int sp_anni1, int sp_anni2,
		       int sp_crea1,
		       int miss_parity, int miss_m, int E);

void created_state(int *in_sp_other,
		   int *in_sp,
#if ANICR2
		   int sp_anni1, int sp_anni2,
		   int sp_crea1, int sp_crea2
#else
		   int sp_anni, int sp_crea
#endif
		   );

void annihilate_states_2nd(int *in_sp_other,
			   int *in_sp,
			   int sp_anni1,
			   int miss_parity, int miss_m, int E);

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
			out_sp, in_sp[0],
			sp_info[in_sp[0]]._l,
			sp_info[in_sp[0]]._m, E);
#else
  create_states(in_sp_other,
		out_sp, in_sp[0], 
		sp_info[in_sp[0]]._l & 1,
		sp_info[in_sp[0]]._m, E);
#endif

  E += SP_STATE_E(sp_info[in_sp[0]]);

  /* And now try with all other missing ones. */

  for (i = 0; i < NSP - 1; i++)
    {
      /* We always have the space at [0] empty. */

      out_sp[i+1] = in_sp[i];

#if ANICR2
      annihilate_states_2nd(in_sp_other,
			    out_sp, in_sp[i+1],
			    sp_info[in_sp[i+1]]._l,
			    sp_info[in_sp[i+1]]._m,
			    E - SP_STATE_E(sp_info[in_sp[i+1]]));
#else
      create_states(in_sp_other,
		    out_sp, in_sp[i+1],
		    sp_info[in_sp[i+1]]._l & 1,
		    sp_info[in_sp[i+1]]._m,
		    E - SP_STATE_E(sp_info[in_sp[i+1]]));
#endif
    }
}

void annihilate_states_2nd(int *in_sp_other,
			   int *in_sp, int sp_anni1,
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

  create_states_1st(in_sp_other,
		    out_sp, sp_anni1, in_sp[1],
		    (sp_info[in_sp[1]]._l ^ miss_parity) & 1,
		    miss_m + sp_info[in_sp[1]]._m,
		    E - SP_STATE_E(sp_info[in_sp[1]]));

  /* And now try with all other missing ones. */

  for (i = 1; i < NSP - 1; i++)
    {
      /* We always have the space at [0] and [1] empty. */

      out_sp[i+1] = in_sp[i];

      create_states_1st(in_sp_other,
			out_sp, sp_anni1, in_sp[i+1],
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
  printf ("------------------------------------------------------------------------------\n");
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
		    (int) crea_sp);
    }


}

void create_states_1st(int *in_sp_other,
		       int *in_sp, int sp_anni1, int sp_anni2,
		       int miss_parity, int miss_m, int E)
{
  int i;

  int out_sp[NSP + 1];

  /* We are missing a certain m, and also have a known
   * energy.
   */

  info_state_for_miss *miss_info = &_table_2_0_info;

  /* Print the state. */

#if DEBUG_ANICR
  printf ("------------------------------------------------------------------------------\n");
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

  int table_end_E = (max_add_E + miss_parity) & ~1;

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
			(int) crea_sp, fill+1,
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

void created_state(int *in_sp_other,
		   int *in_sp,
#if ANICR2
		   int sp_anni1, int sp_anni2,
		   int sp_crea1, int sp_crea2
#else
		   int sp_anni, int sp_crea
#endif
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
#else
  (void) sp_anni;
  (void) sp_crea;
#endif

  int lookfor[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    lookfor[i] = in_sp[i];
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    lookfor[CFG_NUM_SP_STATES0 + i] = in_sp_other[i];

#if DEBUG_ANICR
  for (i = 0; i < CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1; i++)
    printf (" %4d", lookfor[i]);
  printf ("\n");
#endif

  if (!find_mp_state(lookfor))
    {
      printf ("NOT FOUND!\n");

    }
  /* printf ("%4d %4d\n", sp_anni, sp_crea); */
}
