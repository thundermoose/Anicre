#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "anicr_tables.h"
#include "anicr_config.h"

#include "create.h"

#include "tmp_config.h"

#include <gsl/gsl_errno.h>
#include "gsl/gsl_sf_coupling.h"

extern double *_accumulate;

void couple_accumulate()
{
  size_t num_accum;
  size_t non_zero = 0;
  size_t i;

#if ANICR2
  num_accum = CFG_TOT_FIRST_SCND * CFG_TOT_FIRST_SCND;
#else
  num_accum = CFG_NUM_SP_STATES * CFG_NUM_SP_STATES;
#endif

  for (i = 0; i < num_accum; i++)
    {
      if (_accumulate[i])
	non_zero++;
    }
  /*
  for (i = 0; i < num_accum && i < 100; i++)
    {
      printf ("%3zd %.6f\n", i, _accumulate[i]);
    }
  */
  printf ("%zd non-0 accumulate items.\n", non_zero);

  /* double mult = sqrt(CFG_2J_FINAL + 1); */

  int jtrans_min = abs(CFG_2J_INITIAL - CFG_2J_FINAL);
  int jtrans_max = CFG_2J_INITIAL + CFG_2J_FINAL;

  int jtrans;

  int mtrans = CFG_2M_INITIAL - CFG_2M_FINAL;

  if (abs(mtrans) > jtrans_max)
    {
      fprintf (stderr, "FIXME: abs(mtrans) > jtrans_max.\n");
      exit(1);
    }

  for (jtrans = jtrans_min; jtrans <= jtrans_max; jtrans += 2)
    {
      printf ("Jtrans=%d\n", jtrans/2);

      double mult;

      {
	gsl_sf_result result;
	
	int ret =
	  gsl_sf_coupling_3j_e(CFG_2J_INITIAL,  jtrans,  CFG_2J_FINAL,
			       CFG_2M_INITIAL, -mtrans, -CFG_2M_FINAL,
			       &result);
	
	if (ret != GSL_SUCCESS)
	  {
	    fprintf (stderr,"ERR! %d\n", ret);
	    exit(1);
	  }

	int sign = 1 - ((CFG_2J_INITIAL - jtrans + CFG_2M_FINAL) & 2);

	mult = 1 / (result.val) * sign;
      }

      (void) mult;


#if ANICR2
  double final_1b[CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES];
  int sp_anni1;
  int sp_anni2;
  int sp_crea1;
  int sp_crea2;

  memset (final_1b, 0, sizeof (final_1b));

  uint64_t checked = 0;

  for (sp_anni1 = 0; sp_anni1 < CFG_END_JM_FIRST; sp_anni1++)
    for (sp_anni2 = sp_anni1+1; sp_anni2 < CFG_NUM_SP_STATES; sp_anni2++)
      for (sp_crea1 = 0; sp_crea1 < CFG_END_JM_FIRST; sp_crea1++)
	for (sp_crea2 = sp_crea1+1; sp_crea2 < CFG_NUM_SP_STATES; sp_crea2++)
	  {
	    int sp_a = sp_anni1 * (2 * CFG_NUM_SP_STATES - sp_anni1 - 3) / 2 + sp_anni2 - 1;
	    int sp_c = sp_crea1 * (2 * CFG_NUM_SP_STATES - sp_crea1 - 3) / 2 + sp_crea2 - 1;

	    assert (sp_a >= 0 && sp_a < CFG_TOT_FIRST_SCND);
	    assert (sp_c >= 0 && sp_c < CFG_TOT_FIRST_SCND);

	    printf ("a: %3d,%3d  c %3d,%3d   %d %d\n", 
		    sp_anni1, sp_anni2, sp_crea1, sp_crea2,
		    sp_a, sp_c);
	    
	    int acc_i = sp_a * CFG_TOT_FIRST_SCND + sp_c;

	    /*
	  int acc_i = sp_anni * CFG_NUM_SP_STATES + sp_crea;
	    */

	    checked++;

	  if (_accumulate[acc_i])
	    {
	      sp_state_info *sp_a1 = &_table_sp_states[sp_anni1];
	      sp_state_info *sp_a2 = &_table_sp_states[sp_anni2];
	      sp_state_info *sp_c1 = &_table_sp_states[sp_crea1];
	      sp_state_info *sp_c2 = &_table_sp_states[sp_crea2];

	      printf ("a: %3d,%3d  c %3d,%3d"
		      " : %2d %2d,%2d %2d - %2d %2d,%2d %2d [%10.6f]",
		      sp_anni1+1, sp_anni2+1, sp_crea1+1, sp_crea2+1,
		      sp_a1->_j, sp_a1->_m, sp_a2->_j, sp_a2->_m, 
		      sp_c1->_j, sp_c1->_m, sp_c2->_j, sp_c2->_m,
		      _accumulate[acc_i]);

	      /* searching for jtrans */
#if 0
	      int diff_j = abs(sp_a->_j - sp_c->_j);
	      int sum_j  = sp_a->_j + sp_c->_j;
	      int sum_m  = sp_a->_m - sp_c->_m;

	      if (diff_j <= jtrans && sum_j >= jtrans &&
		  abs(sum_m) <= jtrans)
		{
		  printf (" *");

		  gsl_sf_result result;
	  
		  int ret =
		    gsl_sf_coupling_3j_e(sp_a->_j, jtrans,  sp_c->_j,
					 sp_a->_m, -sum_m, -sp_c->_m,
					 &result);

		  if (ret != GSL_SUCCESS)
		    {
		      fprintf (stderr,"ERR! %d\n", ret);
		      exit(1);
		    }

		  int sign = 1 - ((sp_c->_j - jtrans + sp_a->_m) & 2);

		  printf (" [%10.5f %2d]", result.val, sign);

		  printf (" %2d %2d", sp_a->_nlj+1, sp_c->_nlj+1);

		  int fin_i = sp_a->_nlj * CFG_NUM_NLJ_STATES + sp_c->_nlj;

		  final_1b[fin_i] +=
		    result.val * _accumulate[acc_i] * sign;

		}
#endif
	      printf ("\n");
	    }	  
    }

  printf ("%" PRIu64 " checked\n", checked);

  int nlj_a, nlj_c;

  for (nlj_a = 0; nlj_a < CFG_NUM_NLJ_STATES; nlj_a++)
    {
      for (nlj_c = 0; nlj_c < CFG_NUM_NLJ_STATES; nlj_c++)
	{
	  int fin_i = nlj_a * CFG_NUM_NLJ_STATES + nlj_c;
	  
	  if (final_1b[fin_i])
	    {
	      printf ("%3d %3d  %11.6f\n",
		      nlj_a+1, nlj_c+1, mult * final_1b[fin_i]);
	    }


	}
    }


#else
  double final_1b[CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES];
  int sp_anni;
  int sp_crea;

  memset (final_1b, 0, sizeof (final_1b));

  for (sp_anni = 0; sp_anni < CFG_NUM_SP_STATES; sp_anni++)
    {
      for (sp_crea = 0; sp_crea < CFG_NUM_SP_STATES; sp_crea++)
	{
	  int acc_i = sp_anni * CFG_NUM_SP_STATES + sp_crea;

	  if (_accumulate[acc_i])
	    {
	      sp_state_info *sp_a = &_table_sp_states[sp_anni];
	      sp_state_info *sp_c = &_table_sp_states[sp_crea];

	      printf ("a: %3d  c %3d : %2d %2d - %2d %2d [%10.6f]",
		      sp_anni+1, sp_crea+1,
		      sp_a->_j, sp_a->_m, sp_c->_j, sp_c->_m,
		      _accumulate[acc_i]);

	      /* searching for jtrans */

	      int diff_j = abs(sp_a->_j - sp_c->_j);
	      int sum_j  = sp_a->_j + sp_c->_j;
	      int sum_m  = sp_a->_m - sp_c->_m;

	      if (diff_j <= jtrans && sum_j >= jtrans &&
		  abs(sum_m) <= jtrans)
		{
		  printf (" *");

		  gsl_sf_result result;
	  
		  int ret =
		    gsl_sf_coupling_3j_e(sp_a->_j, jtrans,  sp_c->_j,
					 sp_a->_m, -sum_m, -sp_c->_m,
					 &result);

		  if (ret != GSL_SUCCESS)
		    {
		      fprintf (stderr,"ERR! %d\n", ret);
		      exit(1);
		    }

		  int sign = 1 - ((sp_c->_j - jtrans + sp_a->_m) & 2);

		  printf (" [%10.5f %2d]", result.val, sign);

		  printf (" %2d %2d", sp_a->_nlj+1, sp_c->_nlj+1);

		  int fin_i = sp_a->_nlj * CFG_NUM_NLJ_STATES + sp_c->_nlj;

		  final_1b[fin_i] +=
		    result.val * _accumulate[acc_i] * sign;

		}

	      printf ("\n");
	    }	  
	}
    }

  int nlj_a, nlj_c;

  for (nlj_a = 0; nlj_a < CFG_NUM_NLJ_STATES; nlj_a++)
    {
      for (nlj_c = 0; nlj_c < CFG_NUM_NLJ_STATES; nlj_c++)
	{
	  int fin_i = nlj_a * CFG_NUM_NLJ_STATES + nlj_c;
	  
	  if (final_1b[fin_i])
	    {
	      printf ("%3d %3d  %11.6f\n",
		      nlj_a+1, nlj_c+1, mult * final_1b[fin_i]);
	    }


	}
    }

#endif
    }


}
