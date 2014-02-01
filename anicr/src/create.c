
#include "anicr_tables.h"

/* Annihilate states. */

#define NSP 7

void annihilate_states(int *in_sp)
{
  int i;

  /* Delete 1 state. */

  int out_sp[NSP];

  int E = 0;

  for (i = 1; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
      E += sp_info[in_sp[i]]._E;
    }

  /* The out_sp list is missing sp state 0. */

  create_states(out_sp, M - sp_info[in_sp[0]]._m, E);

  int E += sp_info[in_sp[0]]._E;

  /* And now try with all other missing ones. */

  for (i = 0; i < NSP - 1; i++)
    {
      out_sp[i] = in_sp[i];

      create_states(out_sp,
		    M - sp_info[in_sp[i+1]]._m,
		    E - sp_info[in_sp[i+1]]._E);
    }
}

/* Create states, given that we have the list of states that survived
 * the annihilation.
 */

void create_states(int *in_sp, int miss_m, int E)
{
  int i;

  /* We are missing a certain m, and also have a known
   * energy.
   */

  /* Find the list of potential sp states to use. */

  struct *poss_sp = _poss_sp[miss_m][E];
  int num_poss_sp = ;

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

  for ( ; num_poss_sp; --num_poss_sp)
    {  
      while (poss_sp._sp > in_sp[fill])
	{
	  out_sp[fill] = in_sp[fill];
	}

      created_state(out_sp, );
    }


}

