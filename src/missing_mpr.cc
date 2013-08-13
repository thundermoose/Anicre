
#include <stdio.h>
#include <stdint.h>

#include "missing_mpr.hh"

#include "repl_states.hh"

/* Calculate tables of which sp states may be used when a known
 * m is missing, and known how much energy is left to be used.
 */

void missing_mpr_tables(int M, vect_sp_state &sps)
{
  /*
  for (size_t i = 0; i < sps.size(); i++)
    {
      sp_state &sp = sps[i];

      printf ("%4zd: %3d %3d %3d %3d : %3d\n", i,
	      sp._n, sp._l, sp._j, sp._m, 2 * sp._n + sp._l);
    }
  */

  /* TODO: resort the sp states accoring the major N, next m.
   * (Then n, l, j, which do not matter).
   *
   * Reason for m is such that when several sp states can be used form
   * a certain m, N, then better have the list giving the same partial
   * m contribution after each other.  Then the next state to fill
   * will be using the same list a few times, instead of bouncing back
   * and forth.
   */

  int32_t max_sp_N = 0;

  int32_t  max_sp_mpr = 0;
  int32_t  min_sp_mpr = 0;

  for (size_t i = 0; i < sps.size(); i++)
    {
      sp_state &sp = sps[i];

      int N = 2 * sp._n + sp._l;

      if (N > max_sp_N)
	max_sp_N = N;

      if (sp._m > max_sp_mpr)
	max_sp_mpr = sp._m;
      if (sp._m < min_sp_mpr)
	min_sp_mpr = sp._m;
    }

  printf ("max_N: %2d  min_mpr: %2d  max_mpr: %2d\n",
	  max_sp_N, min_sp_mpr, max_sp_mpr);

  // Calculate tables of with sp states that can be used when we are
  // missing a certain m to reach the total sum_m.  Also keep track
  // of how mush energy is needed at each location

  int32_t miss_m_min = M - max_sp_mpr;
  int32_t miss_m_max = M - min_sp_mpr;

  repl_states_by_m_N repl_st(miss_m_min, miss_m_max, max_sp_N);

  for (int32_t miss_m = miss_m_min; miss_m <= miss_m_max; miss_m++)
    {
      // Simply go through all states.

      for (size_t i = 0; i < sps.size(); i++)
	{
	  sp_state &sp = sps[i];

	  if (sp._m == miss_m)
	    {
	      // Has the correct m to fix the situation.
	      // How much energy does it require?

	      int N = 2 * sp._n + sp._l;

	      repl_st.add_entry(miss_m, N);
	    }
	}
    }

  printf ("===================================\n");
  
  repl_st.dump();



  // When calculating what particle can go in as the second last, we
  // must also take into consideration in what state we might leave
  // the system.  We know that the next particle to be added will have
  // a higher index, i.e. add at least as much energy.
  //
  // I.e. if we add a certain amount of energy, we might already
  // know that further additions are futile.  So, we should consult
  // the previous tables to see if there is any possible future.

  int32_t miss_2m_min = M - 2 * max_sp_mpr;
  int32_t miss_2m_max = M - 2 * min_sp_mpr;

  repl_states_by_m_N repl_st2(miss_2m_min, miss_2m_max, max_sp_N * 2);

  for (int32_t miss_m = miss_2m_min; miss_m <= miss_2m_max; miss_m++)
    {
      // Simply go through all states.

      for (size_t i = 0; i < sps.size(); i++)
	{
	  sp_state &sp = sps[i];

	  // We are missing miss_m.  Can the state m handle that together
	  // with the next fill-in?  Is there enough energy for such an
	  // operation?

	  int next_miss_m = miss_m - sp._m;

	  int next_N_min = repl_st.min_N(next_miss_m);

	  if (next_N_min != INT_MAX)
	    {
	      // Has the correct m to fix the situation.
	      // How much energy does it require?

	      int N = 2 * sp._n + sp._l;

	      repl_st2.add_entry(miss_m, N + next_N_min);
	    }
	}
    }

  printf ("===================================\n");

  repl_st2.dump();
}
