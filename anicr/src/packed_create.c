
#include "sse_helper.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "anicr_tables.h"
#include "anicr_config.h"

#include "create.h"
#include "packed_create.h"
#include "mp_states.h"

/* #include "code.h" */

#define DEBUG_ANICR 0

#define SP_STATE_E(sp) (2*(sp)._n+(sp)._l)

#define sp_info _table_sp_states

#define SHIFT_J  28
#define SHIFT_M  23
#define SHIFT_E  18
#define SHIFT_P  17

#define EXTRACT_SP(x)      ((x) & 0x1ffff)
#define GET_P(x)           ((x) >> SHIFT_P)
#define EXTRACT_E(x)       (((x) >> SHIFT_E) & 0x1f)
#define EXTRACT_CMPR_M(x)  (((x) >> SHIFT_M) & 0x1f)
#define EXTRACT_JM(x)      (((x) >> SHIFT_M) & 0x1ff)

void packed_create_states(uint64_t *packed,
			  int sp_anni,
			  int miss_parity, int miss_m, int E);

void packed_created_state(uint64_t *packed,
			  int sp_anni, int sp_crea);

void packed_dump(uint64_t *packed)
{
  int i;

  for (i = 0; i < CFG_NUM_SP_STATES0; i++)
    {
      mp_pack_info *pack_info = &REVNAME(_mp_pack_info)[i];
      int sp = (int) ((packed[pack_info->_word] & pack_info->_mask) >>
                      pack_info->_shift);
      printf (" %3d", sp);
    }
  printf (" :");
  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    {
      mp_pack_info *pack_info = &REVNAME(_mp_pack_info)[i+CFG_NUM_SP_STATES0];
      int sp = (int) ((packed[pack_info->_word] & pack_info->_mask) >>
                      pack_info->_shift);
      printf (" %3d", sp);
    }
  printf ("\n");
}

void packed_annihilate_states(uint64_t *packed)
{
  int i;

  /* Print the state. */

#if DEBUG_ANICR
  printf ("==============================================================================\n");
  packed_dump(packed);
#endif

  /* Find the sum energy (except for first sp state, which is the first
   * that we annihilate).
   */

  /* Since we have to do lookup in an array, this is best done with
   * normal registers.
   */

  int E = 0;

  for (i = 0; i < CFG_NUM_SP_STATES1; i++)
    {
      mp_pack_info *pack_info = &REVNAME(_mp_pack_info)[i+CFG_NUM_SP_STATES0];
      int sp = (int) ((packed[pack_info->_word] & pack_info->_mask) >>
		      pack_info->_shift);
      E += SP_STATE_E(sp_info[sp]);
    }

  for (i = 1; i < CFG_NUM_SP_STATES0; i++)
    {
      mp_pack_info *pack_info = &REVNAME(_mp_pack_info)[i];
      int sp = (int) ((packed[pack_info->_word] & pack_info->_mask) >>
		      pack_info->_shift);
      E += SP_STATE_E(sp_info[sp]);
    }

#if DEBUG_ANICR
  printf ("E=%3d\n", E);
#endif

  /* Now, we have to work with a copy of the state. */

  uint64_t mp_work[CFG_PACK_WORDS];

  for (i = 0; i < CFG_PACK_WORDS; i++)
    mp_work[i] = packed[i];

  /* Kill first state. */

  int sp_anni;

  {
    mp_pack_info *pack_info = &REVNAME(_mp_pack_info)[0];
    sp_anni = (int) ((mp_work[pack_info->_word] & pack_info->_mask) >>
		     pack_info->_shift);
    mp_work[pack_info->_word] &= ~pack_info->_mask;
  }

  packed_create_states(mp_work, sp_anni,
		       sp_info[sp_anni]._l & 1,
		       sp_info[sp_anni]._m, E);

  /* We need to add the energy of the first state. */

  E += SP_STATE_E(sp_info[sp_anni]);

  /* And now try with all other missing ones. */

  int sp_prev_anni = sp_anni;

  for (i = 1; i < CFG_NUM_SP_STATES0; i++)
    {
      /* The space at state 0 is always empty, we move states up.
       * therefore, they wall always fit.
       */

      mp_pack_info *pack_info = &REVNAME(_mp_pack_info)[i];
      sp_anni = (int) ((mp_work[pack_info->_word] & pack_info->_mask) >>
		       pack_info->_shift);
      uint64_t mp_tmp = mp_work[pack_info->_word];
      mp_tmp &= ~pack_info->_mask;
      mp_tmp |= ((uint64_t) sp_prev_anni) << pack_info->_shift;
      mp_work[pack_info->_word] = mp_tmp;

      packed_create_states(mp_work, sp_anni,
			   sp_info[sp_anni]._l & 1,
			   sp_info[sp_anni]._m,
			   E - SP_STATE_E(sp_info[sp_anni]));

      sp_prev_anni = sp_anni;
    }  

}



void packed_create_states(uint64_t *packed,
			  int sp_anni,
			  int miss_parity, int miss_m, int E)
{
  (void) packed;
  (void) sp_anni;
  (void) miss_parity;
  (void) miss_m;
  (void) E;

  int i;

  info_state_for_miss *miss_info = &_table_1_0_info;

  /* Print the state. */

#if DEBUG_ANICR
  printf ("------------------------------------------------------------------------------\n");
  packed_dump(packed);
#endif

#if DEBUG_ANICR
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

  int num_poss_sp = offset_poss_sp_end - offset_poss_sp;

  if (!num_poss_sp)
    return;

  /* Now, we have to work with a copy of the state, as we will be
   * shuffling things around.
   */

  uint64_t mp_work[CFG_PACK_WORDS];

  for (i = 0; i < CFG_PACK_WORDS; i++)
    mp_work[i] = packed[i];

  /* We need to know the sp state at next slot (1), such that we
   * can move up in slot as the sp states to insert grows.
   */

  int sp_next;

  {
    mp_pack_info *pack_info = &REVNAME(_mp_pack_info)[1];
    sp_next = (int) ((mp_work[pack_info->_word] & pack_info->_mask) >>
		     pack_info->_shift);
  }

  int fill = 0;

  mp_pack_info *pack_info_fill = &REVNAME(_mp_pack_info)[fill];

  for ( ; num_poss_sp; --num_poss_sp, poss_sp_ptr++)
    {
      uint32_t poss_sp = *poss_sp_ptr;
      uint32_t crea_sp = EXTRACT_SP(poss_sp);

      while (crea_sp > (uint32_t) sp_next)
        {
	  /* TODO: in case of importance sampling, i.e. not all possible
	   * mp states exist, we may run into a case when the next state
	   * we have cannot fit into the current location.  That means
	   * that we may abort this loop!
	   */

#if DEBUG_ANICR
          printf ("fill %d -> (crea: %d next: %d)\n", fill, crea_sp, sp_next);
#endif

	  /* First move the next state into the work array. */

	  mp_work[pack_info_fill->_word] |=
	    ((uint64_t) sp_next) << pack_info_fill->_shift;

	  fill++;
	  pack_info_fill = &REVNAME(_mp_pack_info)[fill];

	  /* Clear the current fill position. */

	  mp_work[pack_info_fill->_word] &= ~pack_info_fill->_mask;

	  /* And find the next one. */

	  if (fill == CFG_NUM_SP_STATES0-1)
	    sp_next = INT_MAX;
	  else
	    {
	      mp_pack_info *pack_info = pack_info_fill+1;
	      sp_next = (int) ((mp_work[pack_info->_word] & pack_info->_mask) >>
			       pack_info->_shift);
	    }
	}

      if (crea_sp == (uint32_t) sp_next)
	{
#if DEBUG_ANICR
          printf ("%4d x %3d *\n", crea_sp, fill);
#endif
	  continue;
	}

#if DEBUG_ANICR
      printf ("%4d @ %3d\n", crea_sp, fill);
#endif

      /* Insert the state. */

      mp_work[pack_info_fill->_word] |=
	((uint64_t) crea_sp) << pack_info_fill->_shift;

      /* Find it! */

      packed_created_state(mp_work, sp_anni, (int) crea_sp);

      /* Clean it out again. */

      mp_work[pack_info_fill->_word] &= ~pack_info_fill->_mask;
    }






}

void packed_created_state(uint64_t *packed,
			  int sp_anni, int sp_crea)
{
  (void) sp_anni;
  (void) sp_crea;

#if DEBUG_ANICR
  packed_dump(packed);
#endif

  uint64_t lookfor_x;

  find_mp_state_pre(packed, &lookfor_x);
  find_mp_state_prefetch(lookfor_x);

  double val;

  if (!find_mp_state_post(packed, lookfor_x, &val))
    {
      fprintf (stderr, "NOT FOUND!\n");
      exit (1);
    }

}
