
#include <stdio.h>
#include <stdint.h>

#include "missing_mpr.hh"

#include "repl_states.hh"

void odd_even_min_max(int32_t &min, int32_t &max, int32_t oddeven)
{
  min += ((min ^ oddeven) & 1);
  max -= ((max ^ oddeven) & 1);
}

/* Calculate table of which sp states may be used when a known
 * m is missing, and known how much energy is left to be used.
 */

repl_states_by_m_N *
missing_mpr_table(const vect_sp_state &sps,
		  const repl_states_by_m_N *prev_repl_st,
		  int32_t M,
		  int32_t min_sp_mpr,
		  int32_t max_sp_mpr,
		  int32_t max_sp_N,
		  int32_t oddeven,
		  int miss1, int miss2,
		  bool change_pn)
{
  int32_t miss_m_min = M - max_sp_mpr;
  int32_t miss_m_max = M - min_sp_mpr;

  miss_m_min = min_sp_mpr;
  miss_m_max = max_sp_mpr;

  printf ("miss_m_min: %2d  miss_m_max: %2d  oddeven: %2d\n",
	  miss_m_min, miss_m_max, oddeven);

  odd_even_min_max(miss_m_min, miss_m_max, oddeven);

  printf ("miss_m_min: %2d  miss_m_max: %2d  oddeven: %2d\n",
	  miss_m_min, miss_m_max, oddeven);

  repl_states_by_m_N *repl_st =
    new repl_states_by_m_N(miss_m_min, miss_m_max, max_sp_N, miss1, miss2);

  for (int32_t miss_m = miss_m_min; miss_m <= miss_m_max; miss_m += 2)
    {
      // Simply go through all states.

      for (size_t i = 0; i < sps.size(); i++)
	{
	  const sp_state &sp = sps[i];

	  // We are missing miss_m.  Can the state m handle that together
	  // with the next fill-in?  Is there enough energy for such an
	  // operation?

	  int next_miss_m = miss_m - sp._m;

	  for (int parity = 0; parity < (prev_repl_st ? 2 : 1); parity++)
	    {
	      int next_N_min;

	      if (prev_repl_st)
		{
		  next_N_min = prev_repl_st->min_N(parity,
						   next_miss_m,
						   change_pn ? -1 : (int) i);

		  if (next_N_min == INT_MAX)
		    continue;
		}
	      else
		{
		  if (next_miss_m != 0)
		    continue;

		  next_N_min = 0;
		}

	      // Has the correct m to fix the situation.
	      // How much energy does it require?

	      int N = 2 * sp._n + sp._l;

	      repl_st->add_entry((parity + sp._l) & 1,
				 miss_m, N + next_N_min, (int) i);
	    }
	}
    }

  printf ("===================================\n");

  repl_st->dump();

  return repl_st;
}

/* Calculate tables of which sp states may be used, chained to fill up
 * all missing states.  Begin at the end, i.e. which states can fill
 * the last holes.
 */

void missing_mpr_tables(file_output &out,
			int M, int parity, const vect_sp_state &sps,
			int change_pn_at)
{
  (void) parity;

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
      const sp_state &sp = sps[i];

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
  // of how much energy is needed at each location

  repl_states_by_m_N *repl_st1;

  repl_st1 = missing_mpr_table(sps, NULL,
                               M, 1 * min_sp_mpr, 1 * max_sp_mpr,
                               max_sp_N * 1, 1, 1, 0,
			       0); // odd

  // When calculating what particle can go in as the second last, we
  // must also take into consideration in what state we might leave
  // the system.  We know that the next particle to be added will have
  // a higher index, i.e. add at least as much energy.
  //
  // I.e. if we add a certain amount of energy, we might already
  // know that further additions are futile.  So, we should consult
  // the previous tables to see if there is any possible future.

  repl_states_by_m_N *repl_st2;

  repl_st2 = missing_mpr_table(sps, repl_st1,
                               M, 2 * min_sp_mpr, 2 * max_sp_mpr,
                               max_sp_N * 2, 0, 2, 0, // even
			       change_pn_at == 1);

  repl_states_by_m_N *repl_st3;

  repl_st3 = missing_mpr_table(sps, repl_st2,
			       M, 3 * min_sp_mpr, 3 * max_sp_mpr, 
			       max_sp_N * 3, 1, 3, 0, // odd
			       change_pn_at == 2);

  (void) repl_st3;

  repl_st1->write_table(out, false);
  repl_st2->write_table(out, true);
  repl_st3->write_table(out, true);
}
