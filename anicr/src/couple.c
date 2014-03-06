#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#define __USE_XOPEN
#include <math.h>

#include "anicr_tables.h"
#include "anicr_config.h"

#include "create.h"
#include "accumulate.h"

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

	int sign = 1 - ((CFG_2J_INITIAL/* - jtrans*/ + CFG_2M_FINAL) & 2);

	mult = 1 / (result.val) * sign;
      }

      (void) mult;

#define END_J (CFG_MAX_J+1)

#define J_STRIDE (CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES)

#if ANICR2
      double *final_1b = NULL;    

      size_t n_final_1b = (CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES) * 
	(size_t) (CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES) * (END_J * END_J);

      final_1b = (double *) malloc(sizeof (double) * n_final_1b);

      if (!final_1b)
	{
	  fprintf (stderr, "Memory allocation error (%zd bytes).\n",
		   sizeof (double) * n_final_1b);
	  exit(1);
 	}

      printf ("Allocated %zd nlj-items.\n", n_final_1b);

  int sp_anni1;
  int sp_anni2;
  int sp_crea1;
  int sp_crea2;

  memset (final_1b, 0, sizeof (double) * n_final_1b);

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
	    /*
	    printf ("a: %3d,%3d  c %3d,%3d   %d %d\n", 
		    sp_anni1, sp_anni2, sp_crea1, sp_crea2,
		    sp_a, sp_c);
	    */
	    int acc_i = sp_a * CFG_TOT_FIRST_SCND + sp_c;

	    /*
	  int acc_i = sp_anni * CFG_NUM_SP_STATES + sp_crea;
	    */

	    checked++;

	    {
	      double value;
	      
	      uint64_t key =
		(((uint64_t) sp_anni1) <<  0) |
		(((uint64_t) sp_anni2) << 16) |
		(((uint64_t) sp_crea1) << 32) |
		(((uint64_t) sp_crea2) << 48);

	      int has = accumulate_get(key, &value);

	      if (!has)
		value = 0;

	      if (value != _accumulate[acc_i])
		{
		  fprintf (stderr,
			   "Internal error: acc array vs hash table.\n"
			   "%d %d %d %d  %10.5f %10.5f\n",
			   sp_anni1, sp_anni2, sp_crea1, sp_crea2,
			   value, _accumulate[acc_i]);
		  exit(1);
		}
	    }


	  if (_accumulate[acc_i])
	    {
	      sp_state_info *sp_a1 = &_table_sp_states[sp_anni1];
	      sp_state_info *sp_a2 = &_table_sp_states[sp_anni2];
	      sp_state_info *sp_c1 = &_table_sp_states[sp_crea1];
	      sp_state_info *sp_c2 = &_table_sp_states[sp_crea2];

#if DEBUG_ACCUMULATE
	      printf ("a: %3d,%3d  c %3d,%3d"
		      " : %2d %2d,%2d %2d - %2d %2d,%2d %2d [%10.6f]",
		      sp_anni1+1, sp_anni2+1, sp_crea1+1, sp_crea2+1,
		      sp_a1->_j, sp_a1->_m, sp_a2->_j, sp_a2->_m, 
		      sp_c1->_j, sp_c1->_m, sp_c2->_j, sp_c2->_m,
		      _accumulate[acc_i]);
#endif

	      /* We need to connect the annihilated and created states.
	       */

	      int diff_anni_j = abs(sp_a1->_j - sp_a2->_j);
	      int sum_anni_j = sp_a1->_j + sp_a2->_j;
	      int anni_m = sp_a1->_m + sp_a2->_m;

	      int diff_crea_j = abs(sp_c1->_j - sp_c2->_j);
	      int sum_crea_j = sp_c1->_j + sp_c2->_j;
	      int crea_m = sp_c1->_m + sp_c2->_m;

#if DEBUG_ACCUMULATE
	      printf ("\n");
#endif

	      int anni_j;
	      int crea_j;

	      for (anni_j = diff_anni_j; anni_j <= sum_anni_j; anni_j += 2)
		{
		  if ((sp_a1->_nlj == sp_a2->_nlj) && (anni_j & 2))
		    continue;

		  double mult_anni, val_anni;
		  int sign_anni;

      {
	gsl_sf_result result;
	
	int ret =
	  gsl_sf_coupling_3j_e(sp_a1->_j, sp_a2->_j,  anni_j,
			       sp_a1->_m, sp_a2->_m, -anni_m,
			       &result);
	
	if (ret != GSL_SUCCESS)
	  {
	    fprintf (stderr,"ERR! %d\n", ret);
	    exit(1);
	  }

	int sign = 1 - ((sp_a1->_j - sp_a2->_j + anni_m+anni_j) & 2);

	mult_anni = result.val * sign;

	if (mult_anni > 10000. || mult_anni < -10000.0)
	  {
#if DEBUG_ACCUMULATE
	    printf ("\n=== {%d %d %d, %d %d %d} [%11.6f %d] ===\n",
		    sp_a1->_j, sp_a2->_j,  anni_j,
		    sp_a1->_m, sp_a2->_m, -anni_m,
		    result.val, sign);
#endif
	  }

	val_anni = result.val;
	sign_anni = sign;

	if (sp_a1->_nlj == sp_a2->_nlj)
	  mult_anni *= M_SQRT2;

	mult_anni *= sqrt(anni_j + 1); /* sqrt(2*j+1) */
      }

		  for (crea_j = diff_crea_j; crea_j <= sum_crea_j; crea_j += 2)
		    {
		      if ((sp_c1->_nlj == sp_c2->_nlj) && (crea_j & 2))
			continue;
		      
		      double mult_crea, val_crea;
		      int sign_crea;


      {
	gsl_sf_result result;
	
	int ret =
	  gsl_sf_coupling_3j_e(sp_c1->_j, sp_c2->_j,  crea_j,
			       sp_c1->_m, sp_c2->_m, -crea_m,
			       &result);
	
	if (ret != GSL_SUCCESS)
	  {
	    fprintf (stderr,"ERR! %d\n", ret);
	    exit(1);
	  }

	int sign = 1 - ((sp_c1->_j - sp_c2->_j + 2 * crea_m) & 2);

	mult_crea = result.val * sign;

	val_crea = result.val;
	sign_crea = sign;

	if (sp_c1->_nlj == sp_c2->_nlj)
	  mult_crea *= M_SQRT2;

	mult_crea *= sqrt(crea_j + 1); /* sqrt(2*j+1) */
      }

#if DEBUG_ACCUMULATE
		      printf ("%2d %2d - %2d %2d [%10.5f %2d %10.5f %2d] ",
			      anni_j, anni_m, crea_j, crea_m,
			      val_anni, sign_anni, val_crea, sign_crea);
#endif
		      (void) sign_anni;
		      (void) val_anni;
		      (void) sign_crea;
		      (void) val_crea;
		      
	      /* searching for jtrans */

	      int diff_j = abs(anni_j - crea_j);
	      int sum_j  = anni_j + crea_j;
	      int sum_m  = anni_m - crea_m;

	      if (diff_j <= jtrans && sum_j >= jtrans &&
		  abs(sum_m) <= jtrans)
		{
#if DEBUG_ACCUMULATE
		  printf (" *");
#endif

		  gsl_sf_result result;
	  
		  int ret =
		    gsl_sf_coupling_3j_e(anni_j, jtrans,  crea_j,
					 anni_m, -sum_m, -crea_m,
					 &result);

		  if (ret != GSL_SUCCESS)
		    {
		      fprintf (stderr,"ERR! %d\n", ret);
		      exit(1);
		    }

		  int sign = 1/* - ((crea_j - jtrans + anni_m) & 2)*/;

#if DEBUG_ACCUMULATE
		  printf (" [%10.5f %2d]", result.val, sign);
		  
		  printf (" %2d %2d %2d %2d",
			  sp_a1->_nlj+1, sp_a2->_nlj+1,
			  sp_c1->_nlj+1, sp_c2->_nlj+1);
#endif

		  int fin_i = 
		    J_STRIDE * ((anni_j/2) * END_J + (crea_j/2)) +
		    sp_a1->_nlj * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES + 
		    sp_a2->_nlj * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES + 
		    sp_c1->_nlj * CFG_NUM_NLJ_STATES + 
		    sp_c2->_nlj;

		  double value = mult_anni * mult_crea *
                    result.val * _accumulate[acc_i] * sign;

		  final_1b[fin_i] += value;

		  uint64_t key =
		    (((uint64_t) sp_a1->_nlj) <<  0) |
		    (((uint64_t) sp_a2->_nlj) << 11) |
		    (((uint64_t) sp_c1->_nlj) << 22) |
		    (((uint64_t) sp_c2->_nlj) << 33) |
		    (((uint64_t) anni_j) << 44) |
		    (((uint64_t) crea_j) << 51) |
		    (((uint64_t) jtrans) << 58);
		  
		  nlj_add(key, value);
		  
		}
#if DEBUG_ACCUMULATE
		  printf ("\n");
#endif
		    }
		}
	    }	  
    }

  printf ("%" PRIu64 " checked\n", checked);

  int nlj_a1, nlj_a2, nlj_c1, nlj_c2;
  int anni_j, crea_j;

  size_t nz = 0;

  for (anni_j = 0; anni_j < END_J; anni_j++)
  for (crea_j = 0; crea_j < END_J; crea_j++)

  for (nlj_a1 = 0; nlj_a1 < CFG_NUM_NLJ_STATES; nlj_a1++)
    {
  for (nlj_a2 = 0; nlj_a2 < CFG_NUM_NLJ_STATES; nlj_a2++)
    {
      for (nlj_c1 = 0; nlj_c1 < CFG_NUM_NLJ_STATES; nlj_c1++)
	{
      for (nlj_c2 = 0; nlj_c2 < CFG_NUM_NLJ_STATES; nlj_c2++)
	{
	  int fin_i = 
	    J_STRIDE * ((anni_j) * END_J + (crea_j)) +
	    nlj_a1 * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES + 
	    nlj_a2 * CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES + 
	    nlj_c1 * CFG_NUM_NLJ_STATES + 
	    nlj_c2;

	  {
	    double value;
	  
	    uint64_t key =
	      (((uint64_t) nlj_a1) <<  0) |
	      (((uint64_t) nlj_a2) << 11) |
	      (((uint64_t) nlj_c1) << 22) |
	      (((uint64_t) nlj_c2) << 33) |
	      (((uint64_t) anni_j * 2) << 44) |
	      (((uint64_t) crea_j * 2) << 51) |
	      (((uint64_t) jtrans) << 58);

	    int has = nlj_get(key, &value);

	    if (!has)
	      value = 0;

	    if (value != final_1b[fin_i])
	      {
		fprintf (stderr,
			 "Internal error: nlj array vs hash table.\n"
			 "%d %d %d %d  %d %d %d %10.5f %10.5f\n",
			 nlj_a1, nlj_a2, nlj_c1, nlj_c2,
			 anni_j, crea_j, jtrans,
			 value, final_1b[fin_i]);
		exit(1);
	      }



	  }
		  
	  if (final_1b[fin_i])
	    {
	      /*
	      printf ("Create %3d %3d : %2d | Annihilate %3d %3d : %2d = %11.6f\n",
		      nlj_c1+1, nlj_c2+1, crea_j,
		      nlj_a1+1, nlj_a2+1, anni_j,
		      mult * final_1b[fin_i]);
	      */
	      nz++;
	    }


	}
	}
    }
    }

  printf ("nz nlj items: %zd\n", nz);

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

		  int sign = 1 - ((sp_c->_j/* - jtrans*/ + sp_a->_m) & 2);

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

  size_t nz = 0;

  for (nlj_a = 0; nlj_a < CFG_NUM_NLJ_STATES; nlj_a++)
    {
      for (nlj_c = 0; nlj_c < CFG_NUM_NLJ_STATES; nlj_c++)
	{
	  int fin_i = nlj_a * CFG_NUM_NLJ_STATES + nlj_c;
	  
	  if (final_1b[fin_i])
	    {
	      /*
	      printf ("%3d %3d  %11.6f\n",
		      nlj_a+1, nlj_c+1, mult * final_1b[fin_i]);
	      */
	      nz++;
	    }


	}
    }

  printf ("nz nlj items: %zd\n", nz);

#endif
    }


}
