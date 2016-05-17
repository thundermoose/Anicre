
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

#include CFG_FILENAME_CODE_FR_H

#include "tmp_config.h"

/* Annihilate states. */

#define NSP  CFG_NUM_SP_STATES0
#define NSP_OTHER  CFG_NUM_SP_STATES1

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
#if CFG_ANICR_TWO || CFG_ANICR_THREE
  ammend_table(_table_2_0_info._miss,
	       _table_2_0_info._offset[2 * _table_2_0_info._parity_stride]);
#endif
#if CFG_ANICR_THREE
  ammend_table(_table_3_0_info._miss,
	       _table_3_0_info._offset[2 * _table_3_0_info._parity_stride]);
#endif
}


void create_states(int *in_sp_other,
		   int *in_sp,
#if CFG_ANICR_THREE
		   int sp_anni1, int sp_anni2, int sp_anni3, int sp_crea1, int sp_crea2,
		   int fill,
#elif CFG_ANICR_TWO
		   int sp_anni1, int sp_anni2, int sp_crea1,
#if !CFG_ANICR_NP
		   int fill,
#endif
#else
		   int sp_anni,
#endif
		   int phase_i,
		   int miss_parity, int miss_m, 
#if CFG_CONN_TABLES
		   int miss_E
#else
		   int E
#endif
);

void create_states_1st(int *in_sp_other,
		       int *in_sp, 
#if CFG_ANICR_THREE
		       int sp_anni1, int sp_anni2, int sp_anni3, int sp_crea1,
		       int fill,
#else
		       int sp_anni1, int sp_anni2,
#endif
		       int phase_i,
		       int miss_parity, int miss_m,
#if CFG_CONN_TABLES
		       int miss_E
#else
		       int E
#endif
		       );

void create_states_2nd(int *in_sp_other,
		       int *in_sp,
		       int sp_anni1, int sp_anni2, int sp_anni3,
		       int phase_i,
		       int miss_parity, int miss_m,
#if CFG_CONN_TABLES
		       int miss_E
#else
		       int E
#endif
		       );

void created_state(int *in_sp_other,
		   int *in_sp,
#if CFG_ANICR_THREE
		   int sp_anni1, int sp_anni2, int sp_anni3,
		   int sp_crea1, int sp_crea2, int sp_crea3,
#elif CFG_ANICR_TWO
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
			   int miss_parity, int miss_m,
#if CFG_CONN_TABLES
			   int miss_E, int depth
#else
			   int E
#endif
			   );

void annihilate_states_3rd(int *in_sp_other,
			   int *in_sp,
			   int sp_anni1, int sp_anni2,
			   int phase_i,
			   int miss_parity, int miss_m,
#if CFG_CONN_TABLES
			   int miss_E, int depth
#else
			   int E
#endif
			   );

void annihilate_packed_states(uint64_t *packed
#if CFG_CONN_TABLES
			      ,
			      int miss_parity, int miss_m, int miss_E,
			      int depth
#endif
)
{
  int list[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

  packed_to_int_list(list,packed);   //Verkar skapa en lista med alla tillstånd.
   
  annihilate_states(list + CFG_NUM_SP_STATES0, list
#if CFG_CONN_TABLES
		    ,
		    miss_parity, miss_m, miss_E,
		    depth
#endif
		    );
}

int mp_state_in_E(int *in_sp)
{
  int i;

  int E = 0;

  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    {
      E += SP_STATE_E(sp_info[in_sp[i]]);
    }

  return E;
}

int mp_state_in_M(int *in_sp)
{
  int i;

  int M = 0;

  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    {
      M += sp_info[in_sp[i]]._m;
    }

  return M;
}

int sp_comb_E(uint64_t sp)
{
  int E = 0;

  int sp1 = ((sp >>  0) & 0xffff);
  E  = SP_STATE_E(sp_info[sp1]);

#if CFG_ANICR_TWO || CFG_ANICR_THREE
  int sp2 = ((sp >> 16) & 0xffff);
  E += SP_STATE_E(sp_info[sp2]);

#if CFG_ANICR_THREE
  int sp3 = ((sp >> 32) & 0xffff);
  E += SP_STATE_E(sp_info[sp3]);
#endif
#endif

  return E;
}

int sp_comb_M(uint64_t sp)
{
  int M = 0;

  int sp1 = ((sp >>  0) & 0xffff);
  M  = sp_info[sp1]._m;

#if CFG_ANICR_TWO || CFG_ANICR_THREE
  int sp2 = ((sp >> 16) & 0xffff);
  M += sp_info[sp2]._m;

#if CFG_ANICR_THREE
  int sp3 = ((sp >> 32) & 0xffff);
  M += sp_info[sp3]._m;
#endif
#endif

  return M;
}



void annihilate_states(int *in_sp_other,
		       int *in_sp
#if CFG_CONN_TABLES
		       ,
		       int miss_parity, int miss_m, int miss_E,
		       int depth
#endif
		       )
{
  int i;
#if !CFG_CONN_TABLES
  int miss_parity = 0;
  int miss_m = 0;
#endif

  /* Print the state. */

#if DEBUG_ANICR
  printf ("==============================================================================\n");
  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]);
#if !CFG_CONN_TABLES
  printf (" :");
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]);
#endif
  printf ("\n");
#endif

  /* Delete 1 state. */

  int out_sp[NSP];
 
#if CFG_CONN_TABLES
  for (i = 1; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }
#else
  int E = 0;
  /* int M = 0; */

#if !CFG_CONN_TABLES
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    {
      /* M += sp_info[in_sp_other[i]]._m; */
      E += SP_STATE_E(sp_info[in_sp_other[i]]);
#if DEBUG_ANICR
      printf ("1:%3d  %3d %3d\n",
	      E,
	      sp_info[in_sp_other[i]]._m, SP_STATE_E(sp_info[in_sp_other[i]]));
#endif
    }
#endif

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
#endif

  /* The out_sp list is missing sp state 0. */

#if CFG_CONN_TABLES
# if CFG_ANICR_TWO || CFG_ANICR_THREE
  if (SP_STATE_E(sp_info[in_sp[0]]) <= depth)
# else
  if (SP_STATE_E(sp_info[in_sp[0]]) == depth)
# endif
#endif
    {
#if CFG_ANICR_TWO || CFG_ANICR_THREE
      annihilate_states_2nd(in_sp_other,
			    out_sp, in_sp[0],
			    0,
			    (sp_info[in_sp[0]]._l ^ miss_parity) & 1,
			    miss_m + sp_info[in_sp[0]]._m,
#if CFG_CONN_TABLES
			    miss_E + SP_STATE_E(sp_info[in_sp[0]]),
			    depth - SP_STATE_E(sp_info[in_sp[0]])
#else
			    E
#endif
			    );
#else
      create_states(in_sp_other,
		    out_sp,
		    in_sp[0], 0,
		    (sp_info[in_sp[0]]._l ^ miss_parity) & 1,
		    miss_m + sp_info[in_sp[0]]._m,
#if CFG_CONN_TABLES
		    miss_E + SP_STATE_E(sp_info[in_sp[0]])
#else
		    E
#endif
		    );
#endif
    }

#if !CFG_CONN_TABLES
  E += SP_STATE_E(sp_info[in_sp[0]]);
#endif

  /* And now try with all other missing ones. */

  for (i = 0; i < NSP - (CFG_ANICR_THREE ? 3 : 
			 ((CFG_ANICR_TWO && !CFG_ANICR_NP ? 2 : 1))); i++)
    {
      /* We always have the space at [0] empty. */

      out_sp[i+1] = in_sp[i];

#if CFG_CONN_TABLES
# if CFG_ANICR_TWO || CFG_ANICR_THREE
      if (SP_STATE_E(sp_info[in_sp[i+1]]) <= depth)
# else
      if (SP_STATE_E(sp_info[in_sp[i+1]]) == depth)
# endif
#endif
	{
#if CFG_ANICR_TWO || CFG_ANICR_THREE
	  annihilate_states_2nd(in_sp_other,
				out_sp, in_sp[i+1],
				i+1,
				(sp_info[in_sp[i+1]]._l ^ miss_parity) & 1,
				miss_m + sp_info[in_sp[i+1]]._m,
#if CFG_CONN_TABLES
				miss_E + SP_STATE_E(sp_info[in_sp[i+1]]),
				depth - SP_STATE_E(sp_info[in_sp[i+1]])
#else
				E - SP_STATE_E(sp_info[in_sp[i+1]])
#endif
				);
#else
	  create_states(in_sp_other,
			out_sp, 
			in_sp[i+1], i+1,
			(sp_info[in_sp[i+1]]._l ^ miss_parity) & 1,
			miss_m + sp_info[in_sp[i+1]]._m,
#if CFG_CONN_TABLES
			miss_E + SP_STATE_E(sp_info[in_sp[i+1]])
#else
			E - SP_STATE_E(sp_info[in_sp[i+1]])
#endif
			);
#endif
	}
    }
}

#if CFG_ANICR_TWO || CFG_ANICR_THREE
void annihilate_states_2nd(int *in_sp_other,
			   int *in_sp, int sp_anni1,
			   int phase_i,
			   int miss_parity, int miss_m,
#if CFG_CONN_TABLES
			   int miss_E, int depth
#else
			   int E
#endif
			   )
{
  int i;

  /* Print the state. */

#if DEBUG_ANICR
  printf ("===------------------------------------------------------------------------===\n");
  for (i = 1; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]);
#if !CFG_CONN_TABLES
  printf (" :");
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]);
#endif
  printf ("\n");
#endif

#if DEBUG_ANICR
#if CFG_CONN_TABLES
  printf (" : ~E=%3d  ~m=%3d  ~p=%d\n", miss_E, miss_m, miss_parity);
#else
  printf (" : E=%3d  ~m=%3d  ~p=%d\n", E, miss_m, miss_parity);
#endif
#endif

#if !CFG_ANICR_NP
  /* Delete 1 state. */

  int out_sp[NSP];

  for (i = 2; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }

  /* The out_sp list is missing sp state 0 and 1. */

  if (sp_anni1 < in_sp[1])
    {
#if CFG_ANICR_THREE
# if CFG_CONN_TABLES
      if (SP_STATE_E(sp_info[in_sp[1]]) <= depth)
# endif
	annihilate_states_3rd
#else
# if CFG_CONN_TABLES
      if (SP_STATE_E(sp_info[in_sp[1]]) == depth)
# endif
	create_states_1st
#endif
	/**/             (in_sp_other,
			  out_sp, sp_anni1, in_sp[1], phase_i ^ 1,
			  (sp_info[in_sp[1]]._l ^ miss_parity) & 1,
			  miss_m + sp_info[in_sp[1]]._m,
#if CFG_CONN_TABLES
			  miss_E + SP_STATE_E(sp_info[in_sp[1]])
#if CFG_ANICR_THREE
			  ,depth - SP_STATE_E(sp_info[in_sp[1]])
#endif
#else
			  E - SP_STATE_E(sp_info[in_sp[1]])
#endif
			  );
    }

  /* And now try with all other missing ones. */

  for (i = 1; i < NSP - (CFG_ANICR_THREE ? 2 : 1); i++)
    {
      /* We always have the space at [0] and [1] empty. */

      out_sp[i+1] = in_sp[i];

      if (sp_anni1 < in_sp[i+1])
	{
#if CFG_ANICR_THREE
# if CFG_CONN_TABLES
	  if (SP_STATE_E(sp_info[in_sp[i+1]]) <= depth)
# endif
	    annihilate_states_3rd
#else
# if CFG_CONN_TABLES
	  if (SP_STATE_E(sp_info[in_sp[i+1]]) == depth)
# endif
	    create_states_1st
#endif
      /**/               (in_sp_other,
			  out_sp, sp_anni1, in_sp[i+1], phase_i ^ (i+1),
			  (sp_info[in_sp[i+1]]._l ^ miss_parity) & 1,
			  miss_m + sp_info[in_sp[i+1]]._m,
#if CFG_CONN_TABLES
			  miss_E + SP_STATE_E(sp_info[in_sp[i+1]])
#if CFG_ANICR_THREE
			  ,depth - SP_STATE_E(sp_info[in_sp[i+1]])
#endif
#else
			  E - SP_STATE_E(sp_info[in_sp[i+1]])
#endif
			  );
	}
    }
#else
  /* Delete 1 state. */

  int out_sp_other[NSP_OTHER];

  for (i = 1; i < NSP_OTHER; i++)
    {
      out_sp_other[i] = in_sp_other[i];
    }

  /* The out_sp list is missing sp state 0 and 1. */

  create_states_1st(out_sp_other,
		    in_sp, sp_anni1, in_sp_other[0], phase_i ^ 0,
		    (sp_info[in_sp_other[0]]._l ^ miss_parity) & 1,
		    miss_m + sp_info[in_sp_other[0]]._m,
		    E - SP_STATE_E(sp_info[in_sp_other[0]]));

  /* And now try with all other missing ones. */

  for (i = 0; i < NSP_OTHER - 1; i++)
    {
      /* We always have the space at [0] and [1] empty. */

      out_sp_other[i+1] = in_sp_other[i];

      create_states_1st(out_sp_other,
			in_sp, sp_anni1, in_sp_other[i+1], phase_i ^ (i+1),
			(sp_info[in_sp_other[i+1]]._l ^ miss_parity) & 1,
			miss_m + sp_info[in_sp_other[i+1]]._m,
			E - SP_STATE_E(sp_info[in_sp_other[i+1]]));
    }
#endif
}
#endif

#if CFG_ANICR_THREE
void annihilate_states_3rd(int *in_sp_other,
			   int *in_sp,
			   int sp_anni1, int sp_anni2,
			   int phase_i,
			   int miss_parity, int miss_m,
#if CFG_CONN_TABLES
			   int miss_E, int depth
#else
			   int E
#endif
			   )
{
  int i;

  /* Print the state. */

#if DEBUG_ANICR
  printf ("___------------------------------------------------------------------------___\n");
  for (i = 2; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]);
#if !CFG_CONN_TABLES
  printf (" :");
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]);
#endif
  printf ("\n");
#endif

#if DEBUG_ANICR
#if CFG_CONN_TABLES
  printf (" : ~E=%3d  ~m=%3d  ~p=%d\n", miss_E, miss_m, miss_parity);
#else
  printf (" : E=%3d  ~m=%3d  ~p=%d\n", E, miss_m, miss_parity);
#endif
#endif

  /* Delete 1 state. */

  int out_sp[NSP];

  for (i = 3; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }

  /* The out_sp list is missing sp state 0 and 1. */

  if (sp_anni2 < in_sp[2])
    {
      if (SP_STATE_E(sp_info[in_sp[2]]) == depth)
	create_states_2nd(in_sp_other,
			  out_sp, 
			  sp_anni1, sp_anni2, in_sp[2], phase_i ^ 1,
			  (sp_info[in_sp[2]]._l ^ miss_parity) & 1,
			  miss_m + sp_info[in_sp[2]]._m,
#if CFG_CONN_TABLES
			  miss_E + SP_STATE_E(sp_info[in_sp[2]])
#else
			  E - SP_STATE_E(sp_info[in_sp[2]])
#endif
			  );
    }

  /* And now try with all other missing ones. */

  for (i = 2; i < NSP - 1; i++)
    {
      /* We always have the space at [0], [1] and [2] empty. */

      out_sp[i+1] = in_sp[i];

      if (sp_anni2 < in_sp[i+1])
	{
	  if (SP_STATE_E(sp_info[in_sp[i+1]]) == depth)
	    create_states_2nd(in_sp_other,
			      out_sp,
			      sp_anni1, sp_anni2, in_sp[i+1], phase_i ^ (i+1),
			      (sp_info[in_sp[i+1]]._l ^ miss_parity) & 1,
			      miss_m + sp_info[in_sp[i+1]]._m,
#if CFG_CONN_TABLES
			      miss_E + SP_STATE_E(sp_info[in_sp[i+1]])
#else
			      E - SP_STATE_E(sp_info[in_sp[i+1]])
#endif
			      );
	}
    }
}
#endif

/* Create states, given that we have the list of states that survived
 * the annihilation.
 */

void create_states(int *in_sp_other,
		   int *in_sp,
#if CFG_ANICR_THREE
                   int sp_anni1, int sp_anni2, int sp_anni3, int sp_crea1, int sp_crea2,
		   int fill,
#elif CFG_ANICR_TWO
                   int sp_anni1, int sp_anni2, int sp_crea1,
#if !CFG_ANICR_NP
		   int fill,
#endif
#else
                   int sp_anni,
#endif
		   int phase_i,
		   int miss_parity, int miss_m,
#if CFG_CONN_TABLES
		   int miss_E
#else
		   int E
#endif
		   )
{
  int i;

#if !CFG_ANICR_NP
  int out_sp[NSP + 1];
#else
  int out_sp_other[NSP_OTHER + 1];
#endif

  /* We are missing a certain m, and also have a known
   * energy.
   */

  info_state_for_miss *miss_info = &_table_1_0_info;

  /* Print the state. */

#if DEBUG_ANICR
#if CFG_ANICR_THREE
  printf ("--- a %3d a %3d a %3d c %3d c %3d ------------------------------------------\n", sp_anni1, sp_anni2, sp_anni3, sp_crea1, sp_crea2);
#elif CFG_ANICR_TWO
  printf ("--- a %3d a %3d c %3d --------------------------------------------------------\n", sp_anni1, sp_anni2, sp_crea1);
#else
  printf ("--- a %3d --------------------------------------------------------------------\n", sp_anni);
#endif
#endif

#if DEBUG_ANICR
#if CFG_ANICR_TWO || CFG_ANICR_THREE
#if !CFG_ANICR_NP
  for (i = 0; i < fill; i++)
    printf (" %4d", in_sp[i]); 
  for (i = fill + 1; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]);
#else
  for (i = 1; i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]); 
#endif
#else
  for (i = CFG_ANICR_TWO ? 2 : 1; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]); 
#endif
#if CFG_CONN_TABLES
  printf (" : ~E=%3d  ~m=%3d  ~p=%d\n", miss_E, miss_m, miss_parity);
#else
  printf (" : E=%3d  ~m=%3d  ~p=%d\n", E, miss_m, miss_parity);
#endif
#endif

  /* Find the list of potential sp states to use. */

#if CFG_CONN_TABLES
  /* Can be fixed earlier? */

  if (!(miss_m >= miss_info->_m_min &&
	miss_m <  miss_info->_m_min + miss_info->_m_steps))
    return;

#else
  assert (miss_m >= miss_info->_m_min &&
	  miss_m <  miss_info->_m_min + miss_info->_m_steps);
#endif

#if CFG_CONN_TABLES
  int max_add_E = miss_E;
  int min_add_E = miss_E;

  if (min_add_E < 0)
    return; /* cannot add state with negative energy,
	     * we should kill this already at annihilation.
	     */

  if (min_add_E >= miss_info->_num_E)
    min_add_E = miss_info->_num_E - 1;

  int table_begin_E = (min_add_E + miss_parity) / 2;
#else
  int max_add_E = CFG_MAX_SUM_E - E;

  int table_begin_E = 0;
#endif

  if (max_add_E >= miss_info->_num_E)
    max_add_E = miss_info->_num_E - 1;

  int table_end_E = (max_add_E + miss_parity) / 2 + 1;

  int offset_poss_sp =
    miss_info->_offset[miss_info->_parity_stride * miss_parity +
		       miss_info->_m_stride *
		       (miss_m - miss_info->_m_min) / 2 +
		       table_begin_E];
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

#if CFG_ANICR_TWO || CFG_ANICR_THREE
#if !CFG_ANICR_NP
  for (i = 0; i < fill; i++)
    {
      out_sp[i] = in_sp[i];
    }
  for (i = fill+1; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }
#else
  for (i = 1; i < NSP_OTHER; i++)
    {
      out_sp_other[i] = in_sp_other[i];
    }
#endif
#else
  for (i = 1; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }
#endif

  /* Make sure that we do move fill beyond the end. */
#if !CFG_ANICR_NP
  out_sp[NSP] = INT_MAX;
#else
  out_sp_other[NSP_OTHER] = INT_MAX;
#endif

#if (CFG_ANICR_TWO || CFG_ANICR_THREE) && !CFG_ANICR_NP
  /* Skip past states which are smaller than the one already added. */
  
  for ( ; num_poss_sp; --num_poss_sp, poss_sp_ptr++)
    {
      uint32_t poss_sp = *poss_sp_ptr;
      uint32_t crea_sp = EXTRACT_SP(poss_sp);

#if CFG_ANICR_THREE
      if (crea_sp > (uint32_t) sp_crea2)
	break;
#else
      if (crea_sp > (uint32_t) sp_crea1)
	break;
#endif
    }
#else
  int fill = 0;
#endif

#if !CFG_ANICR_NP
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
#if CFG_ANICR_THREE
		    sp_anni1, sp_anni2, sp_anni3, sp_crea1, sp_crea2,
#elif CFG_ANICR_TWO
		    sp_anni1, sp_anni2, sp_crea1,
#else
		    sp_anni,
#endif
		    (int) crea_sp, phase_i ^ fill);
    }
#else
  for ( ; num_poss_sp; --num_poss_sp, poss_sp_ptr++)
    {
      uint32_t poss_sp = *poss_sp_ptr;
      uint32_t crea_sp = EXTRACT_SP(poss_sp);

      while (crea_sp > (uint32_t) out_sp_other[fill+1])
	{
	  out_sp_other[fill] = out_sp_other[fill+1];
	  fill++;
	}

      if (crea_sp == (uint32_t) out_sp_other[fill+1])
	{
#if DEBUG_ANICR
	  printf ("%4d x %3d *\n", crea_sp, fill);
#endif
	  continue;
	}

      out_sp_other[fill] = (int) crea_sp;

#if DEBUG_ANICR
      printf ("%4d @ %3d\n", crea_sp, fill);
#endif

      created_state(out_sp_other,
		    in_sp,
#if CFG_ANICR_THREE
		    sp_anni1, sp_anni2, sp_anni3, sp_crea1, sp_crea2,
#elif CFG_ANICR_TWO
		    sp_anni1, sp_anni2, sp_crea1,
#else
		    sp_anni,
#endif
		    (int) crea_sp, phase_i ^ fill);
    }
#endif


}

#if CFG_ANICR_TWO || CFG_ANICR_THREE
void create_states_1st(int *in_sp_other,
		       int *in_sp,
#if CFG_ANICR_THREE
		       int sp_anni1, int sp_anni2, int sp_anni3, int sp_crea1,
		       int fill,
#else
		       int sp_anni1, int sp_anni2,
#endif
		       int phase_i,
		       int miss_parity, int miss_m,
#if CFG_CONN_TABLES
		       int miss_E
#else
		       int E
#endif
		       )
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
#if CFG_ANICR_THREE
  printf ("--- a %3d a %3d a %3d c %3d --------------------------------------------------\n", sp_anni1, sp_anni2, sp_anni3, sp_crea1);
#elif CFG_ANICR_TWO
  printf ("--- a %3d a %3d --------------------------------------------------------------\n", sp_anni1, sp_anni2);
#else
  printf ("----a %3d --------------------------------------------------------------------\n", sp_anni1);
#endif
#endif

#if DEBUG_ANICR
#if CFG_ANICR_THREE
  for (i = 0; i < fill; i++)
    printf (" %4d", in_sp[i]);
  for (i = fill + 2; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]);
#else
  for (i = (!CFG_ANICR_NP ? 2 : 1); i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]); 
#endif
#if !CFG_CONN_TABLES
  printf (" :");
  for (i = (!CFG_ANICR_NP ? 0 : 1); i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]);
#endif
#if CFG_CONN_TABLES
  printf (" : ~E=%3d  ~m=%3d  ~p=%d\n", miss_E, miss_m, miss_parity);
#else
  printf (" : E=%3d  ~m=%3d  ~p=%d\n", E, miss_m, miss_parity);
#endif
#endif

  /* Find the list of potential sp states to use. */

#if CFG_CONN_TABLES
  /* Can be fixed earlier? */

  if (!(miss_m >= miss_info->_m_min &&
	miss_m <  miss_info->_m_min + miss_info->_m_steps))
    return;

#else
  assert (miss_m >= miss_info->_m_min &&
	  miss_m <  miss_info->_m_min + miss_info->_m_steps);
#endif

#if CFG_CONN_TABLES
  int max_add_E = miss_E;

  if (miss_E < 0)
    return; /* can be fixed earlier? */
#else
  int max_add_E = CFG_MAX_SUM_E - E;
#endif

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
  printf ("max_add_E=%3d -> %d(+%d) states (%d)\n",
	  max_add_E, offset_poss_sp_end - offset_poss_sp,
	  cont_info >> 18,
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

  if (!num_poss_sp && !cont_info)
    return;

  /* Assume we will begin by inserting a lowest state. */

#if CFG_ANICR_THREE
  for (i = 0; i < fill; i++)
    {
      out_sp[i] = in_sp[i];
    }
  for (i = fill+2; i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }
#else
  for (i = (!CFG_ANICR_NP ? 2 : 1); i < NSP; i++)
    {
      out_sp[i] = in_sp[i];
    }
#endif

  /* Make sure that we do move fill beyond the end. */
  out_sp[NSP] = INT_MAX;

#if CFG_ANICR_THREE
  /* Skip past states which are smaller than the one already added. */
  
  for ( ; ; )
    {
      for ( ; num_poss_sp; --num_poss_sp, poss_sp_ptr++)
	{
	  uint32_t poss_sp = *poss_sp_ptr;
	  uint32_t crea_sp = EXTRACT_SP(poss_sp);

	  if (crea_sp > (uint32_t) sp_crea1)
	    goto done_skipping;
	}
      if (!cont_info)
	break;

      num_poss_sp = cont_info >> 18;
      poss_sp_ptr += cont_info & ((1 << 18) - 1);
      cont_info = 0;
    }
 done_skipping:
  ;
#else
  int fill = 0;
#endif

  for ( ; ; )
    {
      for ( ; num_poss_sp; --num_poss_sp, poss_sp_ptr++)
	{
	  uint32_t poss_sp = *poss_sp_ptr;
	  uint32_t crea_sp = EXTRACT_SP(poss_sp);

	  while (crea_sp > (uint32_t) out_sp[fill+(!CFG_ANICR_NP ? 2 : 1)])
	    {
	      out_sp[fill] = out_sp[fill+(!CFG_ANICR_NP ? 2 : 1)];
	      fill++;
	    }

#if DEBUG_ANICR
	  printf ("===---===\n");
#endif

	  if (crea_sp == (uint32_t) out_sp[fill+(!CFG_ANICR_NP ? 2 : 1)])
	    {
#if DEBUG_ANICR
	      printf ("%4d x %3d *\n", crea_sp, fill);
#endif
	      continue;
	    }

	  out_sp[fill] = (int) crea_sp;

#if DEBUG_ANICR
	  printf ("%4d @ %3d\n", crea_sp, fill);
	  fflush(stdout);
#endif

	  create_states(in_sp_other,
			out_sp,
			sp_anni1, sp_anni2,
#if CFG_ANICR_THREE
			sp_anni3, sp_crea1,
#endif
			(int) crea_sp,
#if !CFG_ANICR_NP
			fill+1,
#endif
			phase_i ^ fill,
			(sp_info[crea_sp]._l ^ miss_parity) & 1,
			miss_m - sp_info[crea_sp]._m,
#if CFG_CONN_TABLES
			miss_E - SP_STATE_E(sp_info[crea_sp])
#else
			E + SP_STATE_E(sp_info[crea_sp])
#endif
			);
	}
      if (!cont_info)
	break;

      num_poss_sp = cont_info >> 18;
      poss_sp_ptr += cont_info & ((1 << 18) - 1);
      cont_info = 0;
    }


}
#endif

#if CFG_ANICR_THREE
void create_states_2nd(int *in_sp_other,
		       int *in_sp,
		       int sp_anni1, int sp_anni2, int sp_anni3,
		       int phase_i,
		       int miss_parity, int miss_m,
#if CFG_CONN_TABLES
		       int miss_E
#else
		       int E
#endif
		       )
{
  int i;

  int out_sp[NSP + 1];

  (void) phase_i;

  /* We are missing a certain m, and also have a known
   * energy.
   */

  info_state_for_miss *miss_info = &_table_3_0_info;

  /* Print the state. */

#if DEBUG_ANICR
  printf ("--- a %3d a %3d a %3d --------------------------------------------------------\n", sp_anni1, sp_anni2, sp_anni3);
#endif

#if DEBUG_ANICR
  for (i = 3; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]); 
#if !CFG_CONN_TABLES
  printf (" :");
  for (i = (!CFG_ANICR_NP ? 0 : 1); i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]);
#endif
#if CFG_CONN_TABLES
  printf (" : ~E=%3d  ~m=%3d  ~p=%d\n", miss_E, miss_m, miss_parity);
#else
  printf (" : E=%3d  ~m=%3d  ~p=%d\n", E, miss_m, miss_parity);
#endif
#endif

  /* Find the list of potential sp states to use. */

#if CFG_CONN_TABLES
  /* Can be fixed earlier? */

  if (!(miss_m >= miss_info->_m_min &&
	miss_m <  miss_info->_m_min + miss_info->_m_steps))
    return;

#else
  assert (miss_m >= miss_info->_m_min &&
	  miss_m <  miss_info->_m_min + miss_info->_m_steps);
#endif

#if CFG_CONN_TABLES
  int max_add_E = miss_E;

  if (miss_E < 0)
    return; /* can be fixed earlier? */
#else
  int max_add_E = CFG_MAX_SUM_E - E;
#endif

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
  printf ("max_add_E=%3d -> %d(+%d) states (%d)\n",
	  max_add_E, offset_poss_sp_end - offset_poss_sp,
	  cont_info >> 18,
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

  if (!num_poss_sp && !cont_info)
    return;

  /* Assume we will begin by inserting a lowest state. */

  for (i = 3; i < NSP; i++)
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

	  while (crea_sp > (uint32_t) out_sp[fill+(3)])
	    {
	      out_sp[fill] = out_sp[fill+(3)];
	      fill++;
	    }

#if DEBUG_ANICR
	  printf ("___---___\n");
#endif

	  if (crea_sp == (uint32_t) out_sp[fill+(3)])
	    {
#if DEBUG_ANICR
	      printf ("%4d x %3d *\n", crea_sp, fill);
#endif
	      continue;
	    }

	  out_sp[fill] = (int) crea_sp;

#if DEBUG_ANICR
	  printf ("%4d @ %3d\n", crea_sp, fill);
	  fflush(stdout);
#endif

	  create_states_1st(in_sp_other,
			   out_sp, sp_anni1, sp_anni2, sp_anni3,
			(int) crea_sp,
#if !CFG_ANICR_NP
			fill+1,
#endif
			phase_i ^ fill,
			(sp_info[crea_sp]._l ^ miss_parity) & 1,
			miss_m - sp_info[crea_sp]._m,
#if CFG_CONN_TABLES
			miss_E - SP_STATE_E(sp_info[crea_sp])
#else
			E + SP_STATE_E(sp_info[crea_sp])
#endif
			);
	}
      if (!cont_info)
	break;

      num_poss_sp = cont_info >> 18;
      poss_sp_ptr += cont_info & ((1 << 18) - 1);
      cont_info = 0;
    }


}
#endif

extern double *_accumulate;

extern double _cur_val;

#if CFG_ANICR_ONE
double one_coeff[CFG_NUM_SP_STATES][CFG_NUM_SP_STATES];
#endif

void created_state(int *in_sp_other,
		   int *in_sp,
#if CFG_ANICR_THREE
		   int sp_anni1, int sp_anni2, int sp_anni3,
		   int sp_crea1, int sp_crea2, int sp_crea3,
#elif CFG_ANICR_TWO
		   int sp_anni1, int sp_anni2,
		   int sp_crea1, int sp_crea2,
#else
		   int sp_anni, int sp_crea,
#endif
		   int phase_i
		   )
{
  int i;
  // double one_coeff[CFG_NUM_MP_STATES][CFG_NUM_MP_STATES];
  /* We need to find the created state in the destination hash table.
   * To get its coefficient.
   */

#if DEBUG_ANICR
  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    printf (" %4d", in_sp[i]);
#if !CFG_CONN_TABLES
  printf (" :");
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    printf (" %4d", in_sp_other[i]);
#endif
  printf (" <- find\n");
#endif

#if CFG_ANICR_THREE
  (void) sp_anni3;
  (void) sp_crea3;
#endif
#if CFG_ANICR_TWO || CFG_ANICR_THREE
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

#if CFG_ANICR_TWO
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
#if 0 //DS
  int acc_i = sp_anni * CFG_NUM_SP_STATES + sp_crea;
#endif
#endif

#if !CFG_CONN_TABLES
#if CFG_ANICR_TWO || CFG_ANICR_THREE
  uint64_t key =
    (((uint64_t) sp_anni1) <<  0) |
    (((uint64_t) sp_anni2) << 16) |
    (((uint64_t) sp_crea1) << 32) |
    (((uint64_t) sp_crea2) << 48);
#else
  uint64_t key =
    (((uint64_t) sp_anni) <<  0) |
    (((uint64_t) sp_crea) << 32);
#endif

  uint64_t acc_x;
 
  accumulate_pre(key, &acc_x);   //Key for transition state (e.g. a+a)
  accumulate_prefetch_rw(acc_x);
  
#endif

  /*
  int lookfor[CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1];

  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    lookfor[i] = in_sp[i];
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    lookfor[CFG_NUM_SP_STATES0 + i] = in_sp_other[i];
  */

  (void) i;

  uint64_t lookfor_packed[CFG_PACK_WORDS];

  int_list2_to_packed(lookfor_packed, in_sp, in_sp_other);  //Created many-body state

  /*
#if DEBUG_ANICR
  for (i = 0; i < CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1; i++)
    printf (" %4d", lookfor[i]);
  printf ("\n");
#endif
  */

  uint64_t lookfor_x;
  find_mp_state_pre(lookfor_packed, &lookfor_x);   //Find possible initia and final states      
  find_mp_state_prefetch(lookfor_x);

#if CFG_CONN_TABLES
  uint64_t ind;

  if (!find_mp_state_post(lookfor_packed, lookfor_x, &ind))
    {
      fprintf (stderr, "NOT FOUND!\n");
      exit (1);
    }

  (void) phase_i;


#else
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


#if DEBUG_COUPLING
  /*Print coefficients for all connections*/
#if CFG_ANICR_TWO
  printf("Coefficients: a1=%4d a2=%4d c1=%4d c2=%4d : bmpi=  %11.8f bmpf=  %11.8f sign= %3d\n",sp_anni1,sp_anni2,sp_crea1,sp_crea2,val,_cur_val,sign);
#else
  printf("Coefficients: a=%4d c=%4d : bmpi=  %11.8f bmpf=  %11.8f sign= %3d\n",sp_anni,sp_crea,val,_cur_val,sign);
#endif
#endif
#if CFG_ANICR_TWO
#if !CFG_CONN_TABLES
  accumulate_advance_add(key, &acc_x);
 
  accumulate_post_add(acc_x, val * _cur_val * sign);
#endif
#elif CFG_ANICR_ONE
  one_coeff[sp_anni][sp_crea]+=val*_cur_val*sign;
#endif
  
#if DEBUG_ANICR
  /* printf ("%5d %15.10f\n", acc_i, val * _cur_val * sign); */
  printf ("%15.10f\n", val * _cur_val * sign);
#endif
#endif
}

