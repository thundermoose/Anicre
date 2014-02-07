
#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "anicr_tables.h"
#include "anicr_config.h"

#include "create.h"

/* Annihilate states. */

#define NSP  CFG_NUM_SP_STATES0

#define SP_STATE_E(sp) (2*(sp)._n+(sp)._l)

#define sp_info _table_sp_states

void create_states(int *in_sp_other,
		   int *in_sp, int sp_anni, int miss_m, int E);

void created_state(int *in_sp_other,
		   int *in_sp, int sp_anni, int sp_crea);

void annihilate_states(int *in_sp_other,
		       int *in_sp)
{
  int i;

  /* Print the state. */

  printf ("==============================================================================\n");
  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]);
  printf (" :");
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]);
  printf ("\n");

  /* Delete 1 state. */

  int out_sp[NSP];

  int E = 0;
  /* int M = 0; */

  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    {
      /* M += sp_info[in_sp_other[i]]._m; */
      E += SP_STATE_E(sp_info[in_sp_other[i]]);
      printf ("%3d  %3d %3d\n",
	      E,
	      sp_info[in_sp_other[i]]._m, SP_STATE_E(sp_info[in_sp_other[i]]));
    }

  for (i = 1; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
      /* M += sp_info[in_sp[i]]._m; */
      E += SP_STATE_E(sp_info[in_sp[i]]);
      printf ("%3d  %3d %3d\n",
	      E,
	      sp_info[in_sp[i]]._m, SP_STATE_E(sp_info[in_sp[i]]));
    }

  printf ("E=%3d\n", E);

  /* The out_sp list is missing sp state 0. */

  create_states(in_sp_other,
		out_sp, in_sp[0], - sp_info[in_sp[0]]._m, E);

  E += SP_STATE_E(sp_info[in_sp[0]]);

  /* And now try with all other missing ones. */

  for (i = 0; i < NSP - 1; i++)
    {
      /* We always have the space at [0] empty. */

      out_sp[i+1] = in_sp[i];

      create_states(in_sp_other,
		    out_sp, in_sp[i+1],
		    - sp_info[in_sp[i+1]]._m,
		    E - SP_STATE_E(sp_info[in_sp[i+1]]));
    }
}

/* Create states, given that we have the list of states that survived
 * the annihilation.
 */

void create_states(int *in_sp_other,
		   int *in_sp, int sp_anni, int miss_m, int E)
{
  int i;

  int out_sp[NSP + 1];

  /* We are missing a certain m, and also have a known
   * energy.
   */

  info_state_for_miss *miss_info = &_table_1_0_info;

  /* Print the state. */

  printf ("------------------------------------------------------------------------------\n");

  for (i = 1; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]); 
  printf (" : E=%3d  ~m=%3d\n", E, miss_m);

  /* Find the list of potential sp states to use. */

  assert (miss_m >= miss_info->_m_min &&
	  miss_m <  miss_info->_m_min + miss_info->_m_steps);

  int max_add_E = CFG_MAX_SUM_E - E;

  if (max_add_E >= miss_info->_num_E)
    max_add_E = miss_info->_num_E - 1;

  int offset_poss_sp =
    miss_info->_offset[miss_info->_num_E * (miss_m - miss_info->_m_min) / 2];
  int offset_poss_sp_end =
    miss_info->_offset[miss_info->_num_E * (miss_m - miss_info->_m_min) / 2 +
		       max_add_E + 1];

  printf ("max_add_E=%3d -> %d states (%d)\n",
	  max_add_E, offset_poss_sp_end - offset_poss_sp,
	  offset_poss_sp);

  state_for_miss_m_N *poss_sp/*, *poss_sp_end*/;

  poss_sp = &miss_info->_miss[offset_poss_sp];
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

  for (i = 0; i < NSP-1; i++)
    {
      out_sp[i+1] = in_sp[i];
    }

  /* Make sure that we do move fill beyond the end. */
  out_sp[NSP] = INT_MAX;

  int fill = 0;

  for ( ; num_poss_sp; --num_poss_sp, poss_sp++)
    {
      while (*poss_sp > in_sp[fill+1])
	{
	  out_sp[fill] = in_sp[fill+1];
	  fill++;
	}

      if (*poss_sp == in_sp[fill+1])
	{
	  printf ("%4d x %3d\n", *poss_sp, fill+1);
	  continue;
	}

      out_sp[fill] = *poss_sp;

      printf ("%4d @ %3d\n", *poss_sp, fill);

      created_state(in_sp_other,
		    out_sp, sp_anni, *poss_sp);
    }


}

void created_state(int *in_sp_other,
		   int *in_sp, int sp_anni, int sp_crea)
{
  /* We need to find the created state in the destination hash table.
   * To get its coefficient.
   */

  (void) in_sp_other;
  (void) in_sp;
  (void) sp_anni;
  (void) sp_crea;
}
