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

#if ACC_TABLE
extern double *_accumulate;
#endif

extern double one_coeff[CFG_NUM_SP_STATES][CFG_NUM_SP_STATES];
#if !CFG_CONN_TABLES
void couple_accumulate()
{

#if ACC_TABLE
  size_t num_accum;
  size_t non_zero = 0;
  size_t i;

#if CFG_ANICR_TWO
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
#endif

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
  FILE *fp=NULL;
  fp=fopen(CFG_FILENAME_NLJ,"wb");
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

#if CFG_ANICR_TWO

#else
      double final_1b[CFG_NUM_NLJ_STATES * CFG_NUM_NLJ_STATES];
      int sp_anni;
      int sp_crea;

      memset (final_1b, 0, sizeof (final_1b));

      for (sp_anni = 0; sp_anni < CFG_NUM_SP_STATES; sp_anni++)
	{
	  for (sp_crea = 0; sp_crea < CFG_NUM_SP_STATES; sp_crea++)
	    {
	  //int acc_i = sp_anni * CFG_NUM_SP_STATES + sp_crea;

	      if (one_coeff[sp_anni][sp_crea])
		{
		  sp_state_info *sp_a = &_table_sp_states[sp_anni];
		  sp_state_info *sp_c = &_table_sp_states[sp_crea];
#if DEBUG_ANICR
		  printf ("a: %3d  c %3d : %2d %2d - %2d %2d [%10.6f]",
			  sp_anni+1, sp_crea+1,
			  sp_a->_j, sp_a->_m, sp_c->_j, sp_c->_m,
			  one_coeff[sp_anni][sp_crea]);
#endif
	      /* searching for jtrans */
		  
		  int diff_j = abs(sp_a->_j - sp_c->_j);
		  int sum_j  = sp_a->_j + sp_c->_j;
		  int sum_m  = sp_a->_m - sp_c->_m;

		  if (diff_j <= jtrans && sum_j >= jtrans &&
		      abs(sum_m) <= jtrans)
		    {
#if DEBUG_ANICR
		      printf (" *");
#endif 
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

		      int sign = 1 - ((sp_c->_j/* - jtrans*/ + sp_a->_m) & 2);   //kontrollerA!
#if DEBUG_ANICR
		      printf (" [%10.5f %2d]", result.val, sign);
		      
		      printf (" %2d %2d", sp_a->_nlj+1, sp_c->_nlj+1);
#endif
		      int fin_i = sp_a->_nlj * CFG_NUM_NLJ_STATES + sp_c->_nlj;

		      final_1b[fin_i] +=
			result.val * one_coeff[sp_anni][sp_crea] * sign;   //one_coeff can be hash-table to save memory

		    }
#if DEBUG_ANICR
		  printf ("\n");
#endif
		}	  
	    }
	}

      
      size_t nz = 0;
#if DEBUG_ANICR
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
		  
		  nz++;
		}


	    }
	}
#endif
      if(fp!=NULL){
	fwrite(final_1b, sizeof(double),CFG_NUM_NLJ_STATES*CFG_NUM_NLJ_STATES,fp);
      }  
      else{
	printf("Couldn't write to file");
	exit(0);

      }
      printf ("nz nlj items: %zd\n", nz);

#endif
    }
  fclose(fp);
  //#endif
}



typedef struct couple_item_t
{
  uint64_t _nlj_key;
  int      _fact_anni_crea; /* x1: anni_nlj_same, x2: crea_nlj_same */
  double   _value;

} couple_item;

couple_item *_couple_items = NULL;

void alloc_couple_items(size_t max_anni, size_t max_crea)
{
  size_t max = max_anni * max_crea;

  _couple_items = (couple_item *) malloc (sizeof (couple_item) * max);

  if (!_couple_items)
    {
      fprintf (stderr, "Memory allocation error (_couple_items).\n");
      exit(1);
    }
}

typedef struct couple_j_item_t
{
  int _j;
  double _val;
} couple_j_item;

#if CFG_ANICR_TWO
void couple_accumulate_2()
{
  /* Figure out jtrans limits. */

  int jtrans_min = abs(CFG_2J_INITIAL - CFG_2J_FINAL);
  int jtrans_max = CFG_2J_INITIAL + CFG_2J_FINAL;

  int mtrans = CFG_2M_INITIAL - CFG_2M_FINAL;

  if (abs(mtrans) > jtrans_max)
    {
      fprintf (stderr, "FIXME: abs(mtrans) > jtrans_max.\n");
      exit(1);
    }

  /* */ 

  uint64_t had = 0, nz_had = 0;

  /* Loop over all combinations of annihilation j,m j,m
   */

  size_t a_i;

  /* We use the list sorted by sum_m, since then we'll have a higher
   * chance to close by reuse the lists for sum_m for the creation as
   * well.
   */

  couple_j_item anni_items[(CFG_MAX_J+1)];
  couple_j_item crea_items[(CFG_MAX_J+1)];

  for (a_i = 0; a_i < _num_jm_pair_groups; a_i++)
    {
      jm_pair_group *apg = &_jm_pair_groups[a_i];

      /* All sp pairs in the group have the same j,m j,m.
       * so we can precalculate the 3j symbols.
       */

      int diff_anni_j = abs(apg->_info._j[0] - apg->_info._j[1]);
      int sum_anni_j  = apg->_info._j[0] + apg->_info._j[1];
      int anni_m      = apg->_info._m[0] + apg->_info._m[1];

      couple_j_item *end_anni = anni_items;

      if (diff_anni_j < abs(anni_m))
	diff_anni_j = abs(anni_m);

      {
      int anni_j;
      for (anni_j = diff_anni_j; anni_j <= sum_anni_j; anni_j += 2)
	{
	  double mult_anni/*, val_anni*/;
	  /* int sign_anni; */

	  gsl_sf_result result;
        
	  int ret =
	    gsl_sf_coupling_3j_e(apg->_info._j[0], apg->_info._j[1],  anni_j,
				 apg->_info._m[0], apg->_info._m[1], -anni_m,
				 &result);
	  
	  if (ret != GSL_SUCCESS)
	    {
	      fprintf (stderr,"ERR! %d\n", ret);
	      exit(1);
	    }

	  int sign =
	    1 - ((apg->_info._j[0] - apg->_info._j[1] + anni_m) & 2);    //DS

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
	 
	  /*
	  val_anni = result.val;
	  sign_anni = sign;
	  */
	  /*
	  if (sp_a1->_nlj == sp_a2->_nlj)
	    mult_anni *= M_SQRT2;
	  */

	  mult_anni *= sqrt(anni_j + 1); /* sqrt(2*j+1) */

	  end_anni->_j   = anni_j;
	  end_anni->_val = mult_anni;
	  end_anni++;

	  assert ((size_t) (end_anni - anni_items) <=
		  sizeof(anni_items) / sizeof(anni_items[0]));
	}
      }

      /* Then, loop over the possible combinations of creation j,m j,m
       * Since we have removed a certain sum_m, only some groups
       * can come into account, those adding the appropriate m.
       */

      int sum_m_want = CFG_2M_FINAL - CFG_2M_INITIAL + anni_m;
      
      int idx = (sum_m_want - _summ_parity_jm_pair_groups_min_sum_m) / 2;

      jm_pair_group *begin = _summ_parity_jm_pair_groups[idx];
      jm_pair_group *end   = _summ_parity_jm_pair_groups[idx+1];
      jm_pair_group *cpg;

      for (cpg = begin; cpg != end; cpg++)
	{
	  /*
	  printf ("a_i: %zd  c_i: %zd\n", a_i, cpg - _jm_pair_groups);
	  */
	  /* We will always have at least one member?
	   * For initial = final this is true, but not otherwise...
	   * So perheps we should look for matching accumulation
	   * items before we start to calculate the 3j symbol values.
	   */

	  couple_item *end_items = _couple_items;

	  /* Loop over the annihilation items.  For each parity, as that
	   * determines the possible parity of the creation items.
	   */

	  int anni_parity;

	  for (anni_parity = 0; anni_parity < 2; anni_parity++)
	    {
	      int crea_parity =
		CFG_PARITY_FINAL ^ CFG_PARITY_INITIAL ^ anni_parity;
	      /*
	      printf ("  a_p: %d  c_p: %d\n", anni_parity, crea_parity);
	      */
	      uint32_t *anni_list = apg->_pairs[anni_parity];
	      uint32_t anni_num   = apg->_num[anni_parity];

	      for ( ; anni_num; anni_num--)
		{
		  uint32_t anni_pair = *(anni_list++);
		  uint32_t anni_nlj = *(anni_list++);

		  uint32_t *crea_list = cpg->_pairs[crea_parity];
		  uint32_t crea_num   = cpg->_num[crea_parity];

		  for ( ; crea_num; crea_num--)
		    {
		      uint32_t crea_pair = *(crea_list++);
		      uint32_t crea_nlj = *(crea_list++);

		      uint64_t acc_key =
			anni_pair | (((uint64_t) crea_pair) << 32);

		      double value;

		      uint64_t x;

		      accumulate_pre(acc_key, &x);
		      accumulate_prefetch_r(x);
		      int has = accumulate_post_get(acc_key, x, &value);

		      if (!has)
			{
			  fprintf (stderr, "Internal error, "
				   "missing anni-crea item %016"PRIx64".\n",
				   acc_key);
			  exit(1);
			}

		      had++;
		      /*
		      printf ("%016"PRIx64"\n", acc_key);
		      */
		      /* No need to work on zero items. */

		      if (value)
			{
			  uint64_t nlj_key =
			    (anni_nlj & ((1 << 22) - 1)) |
			    (((uint64_t) (crea_nlj & ((1 << 22) - 1))) << 22);

			  end_items->_nlj_key = nlj_key;
			  end_items->_value = value;

#if !CFG_ANICR_NP
			  int fact_anni_crea =
			    (int) (((anni_nlj >> 22) & 1) |
				   ((crea_nlj >> 21) & 2));
#else
			  int fact_anni_crea = 0;
#endif

			  end_items->_fact_anni_crea = fact_anni_crea;

			  end_items++;

			  nz_had++;
			}
		    }
		}
	    }

	  if (end_items == _couple_items)
	    continue;

	  /* We have items.  Find the 3j's that can come into play. */

	  int diff_crea_j = abs(cpg->_info._j[0] - cpg->_info._j[1]);
	  int sum_crea_j  = cpg->_info._j[0] + cpg->_info._j[1];
	  int crea_m      = cpg->_info._m[0] + cpg->_info._m[1];

	  couple_j_item *end_crea = crea_items;

	  if (diff_crea_j < abs(anni_m))
	    diff_crea_j = abs(anni_m);

	  {
	  int crea_j;
	  for (crea_j = diff_crea_j; crea_j <= sum_crea_j; crea_j += 2)
	    {
	      double mult_crea/*, val_crea*/;
	      /* int sign_crea; */

	      gsl_sf_result result;
        
	      int ret =
		gsl_sf_coupling_3j_e(cpg->_info._j[0],cpg->_info._j[1], crea_j,
				     cpg->_info._m[0],cpg->_info._m[1],-crea_m,
				     &result);
	  
	      if (ret != GSL_SUCCESS)
		{
		  fprintf (stderr,"ERR! %d\n", ret);
		  exit(1);
		}

	      int sign =
		1 - ((cpg->_info._j[0] - cpg->_info._j[1] +  crea_m) & 2);   //DS:Crea_m or 2*Crea_m

	      mult_crea = result.val * sign;
	  
	  if (mult_crea > 10000. || mult_crea < -10000.0)
	    {
#if DEBUG_ACCUMULATE 
	      printf ("\n=== {%d %d %d, %d %d %d} [%11.6f %d] ===\n",
		      sp_a1->_j, sp_a2->_j,  crea_j,
		      sp_a1->_m, sp_a2->_m, -crea_m,
		      result.val, sign);
#endif
	    }
	  
	  /*
	  val_crea = result.val;
	  sign_crea = sign;
	  */
	  /*
	  if (sp_c1->_nlj == sp_c2->_nlj)
	    mult_crea *= M_SQRT2;
	  */
	  mult_crea *= sqrt(crea_j + 1); /* sqrt(2*j+1) */
	      
	  end_crea->_j   = crea_j;
	  end_crea->_val = mult_crea;
	  end_crea++;
	  
	  assert ((size_t) (end_crea - crea_items) <=
		  sizeof(crea_items) / sizeof(crea_items[0]));
	    }
	  }

	  /* Now we are ready to do the double-loop over the anni-crea.
	   * And for each, we have a loop over the possible jtrans...
	   */

	  couple_j_item *anni_item;
	  couple_j_item *crea_item;

	  for (anni_item = anni_items; anni_item < end_anni; anni_item++)
	    for (crea_item = crea_items; crea_item < end_crea; crea_item++)
	      {
		/* searching for jtrans */
		
		int diff_j = abs(anni_item->_j - crea_item->_j);
		int sum_j  = anni_item->_j + crea_item->_j;
		int sum_m  = anni_m - crea_m;

		int min_j = abs(sum_m);
		int max_j = sum_j;

		if (min_j < diff_j)
		  min_j = diff_j;
		
		if (min_j < jtrans_min)
		  min_j = jtrans_min;
		if (max_j > jtrans_max)
		  max_j = jtrans_max;
		/* Move to dumpnlj:
		double anni_crea_factor[4];

		double anni_factor = (anni_item->_j & 2) ? 0 : M_SQRT2;      
		double crea_factor = (crea_item->_j & 2) ? 0 : M_SQRT2;  

		anni_crea_factor[0] = 1;
		anni_crea_factor[1] = anni_factor;
		anni_crea_factor[2] = crea_factor;
		anni_crea_factor[3] = anni_factor * crea_factor;
		*/
		int jtrans;

		for (jtrans = min_j; jtrans <= max_j; jtrans += 2)
		  {
		    /* Get the coupling. */

		    gsl_sf_result result;
		    
		    int ret =
		      gsl_sf_coupling_3j_e(anni_item->_j,jtrans,crea_item->_j,   //DS:-sum_m=sum_m
					   anni_m,       sum_m,-crea_m,
					   &result);

		    if (ret != GSL_SUCCESS)
		      {
			fprintf (stderr,"ERR! %d\n", ret);
			exit(1);
		      }

		    int sign = 1 - ((anni_item->_j - jtrans + crea_m) & 2);   

		    double coupling =
		      result.val * anni_item->_val * crea_item->_val*sign;
		    uint64_t key_jjj =
		      (((uint64_t) anni_item->_j) << 44) |
		      (((uint64_t) crea_item->_j) << 51) |
		      (((uint64_t) jtrans) << 58);
 
		    /* And now, apply this to all items in the list! */

		    couple_item *items;
		    for (items = _couple_items; items != end_items; items++)
		      {
			uint64_t key =
			  items->_nlj_key | key_jjj;

			//	double factor =anni_crea_factor[items->_fact_anni_crea];
		       
			nlj_add(key, items->_value * coupling);
		      }
		  }
	      }
	}
    }

  printf ("%" PRIu64 " had, %" PRIu64 " nz\n", had, nz_had);
}
#endif
#endif
