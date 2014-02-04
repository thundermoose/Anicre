
#include <assert.h>
#include <limits.h>

#include "anicr_tables.h"

/* Annihilate states. */

#define NSP 7

#define SP_STATE_E(sp) (2*(sp)._n+(sp)._l)

#define sp_info _table_sp_states

void create_states(int *in_sp, int sp_anni, int miss_m, int E);

void created_state(int *in_sp, int sp_anni, int sp_crea);

void annihilate_states(int *in_sp)
{
  int i;

  /* Delete 1 state. */

  int out_sp[NSP];

  int E = 0;
  int M = 0;

  for (i = 1; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
      M += sp_info[in_sp[0]]._m;
      E += SP_STATE_E(sp_info[in_sp[i]]);
    }

  /* The out_sp list is missing sp state 0. */

  create_states(out_sp, in_sp[0], M - sp_info[in_sp[0]]._m, E);

  M += sp_info[in_sp[0]]._m;
  E += SP_STATE_E(sp_info[in_sp[0]]);

  /* And now try with all other missing ones. */

  for (i = 0; i < NSP - 1; i++)
    {
      /* We always have the space at [0] empty. */

      out_sp[i+1] = in_sp[i];

      create_states(out_sp, in_sp[i+1],
		    M - sp_info[in_sp[i+1]]._m,
		    E - SP_STATE_E(sp_info[in_sp[i+1]]));
    }
}

/* Create states, given that we have the list of states that survived
 * the annihilation.
 */

void create_states(int *in_sp, int sp_anni, int miss_m, int E)
{
  int i;

  int out_sp[NSP + 1];

  /* We are missing a certain m, and also have a known
   * energy.
   */

  info_state_for_miss *miss_info = &_table_1_0_info;

  /* Find the list of potential sp states to use. */

  assert (miss_m < miss_info->_m_min + miss_info->_m_steps);

  int offset_poss_sp =
    miss_info->_offset[(miss_m - miss_info->_m_min) / 2 + E];
  int offset_poss_sp_end =
    miss_info->_offset[(miss_m - miss_info->_m_min) / 2 +
		       miss_info->_num_E];

  state_for_miss_m_N *poss_sp/*, *poss_sp_end*/;

  poss_sp = &miss_info->_miss[offset_poss_sp];
  /* poss_sp_end = &miss_info->_miss[offset_poss_sp_end]; */

  int num_poss_sp = offset_poss_sp_end - offset_poss_sp;

  /* Now work through the list. 
   *
   * The list is sorted such that the states come in order, and
   * so is the in_sp list.  I.e. we are performing a merge sort.
   */

  /* Assume we will begin by inserting a lowest state. */

  for (i = 0; i < NSP-1; i++)
    {
      out_sp[i+1] = in_sp[i];
    }

  /* Make sure that we do move fill beyond the end. */
  out_sp[NSP] = INT_MAX;

  int fill = 0;

  for ( ; num_poss_sp; --num_poss_sp)
    {  
      while (*poss_sp > in_sp[fill+1])
	{
	  out_sp[fill] = in_sp[fill+1];
	  fill++;
	}
      out_sp[fill] = *poss_sp;

      created_state(out_sp, sp_anni, *poss_sp);
    }


}

void created_state(int *in_sp, int sp_anni, int sp_crea)
{

  (void) in_sp;
  (void) sp_anni;
  (void) sp_crea;
}
