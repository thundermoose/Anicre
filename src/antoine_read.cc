
#include "antoine_read.hh"
#include "colourtext.hh"

#include "mr_config.hh"

#include <string.h>

extern int _debug;

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

template<class header_version_t>
mr_antoine_reader<header_version_t>::
mr_antoine_reader(mr_file_reader *file_reader)
  : mr_base_reader(file_reader)
{
  _nr_ll_jj = NULL;
  _num_mpr = NULL;
  for (int i = 0; i < 2; i++)
    _occ_used[i] = NULL;
  _jm_used = NULL;
}

template<class header_version_t>
mr_antoine_reader<header_version_t>::~mr_antoine_reader()
{
  free(_nr_ll_jj);
  free(_num_mpr);
  for (int i = 0; i < 2; i++)
    free(_occ_used[i]);
  free(_jm_used);
}


template<class header_version_t>
const char *mr_antoine_reader<header_version_t>::get_format_name() 
{ 
  if (sizeof(header_version_t) == sizeof(mr_antoine_header_old_t))
    return "ANTOINE_OLD";
  else
    return "ANTOINE_NEW";
}

template<class header_version_t>
bool mr_antoine_reader<header_version_t>::level1_read()
{
  uint64_t cur_offset = 0;
  
  TRY_GET_FORTRAN_BLOCK(_header);
  
  CHECK_REASONABLE_RANGE(_header.num_of_shell,1,1000);
  CHECK_REASONABLE_RANGE(_header.num_of_jm,1,10000);
  CHECK_REASONABLE_RANGE_0(_header.A[0],40);
  CHECK_REASONABLE_RANGE_0(_header.A[1],40);
  
  TRY_HAS_FORTRAN_BLOCK_ITEMS(mr_antoine_nr_ll_jj_item_t,
			      _header.num_of_shell,
			      _offset_nr_ll_jj);
  TRY_HAS_FORTRAN_BLOCK_ITEMS(mr_antoine_num_mpr_item_t,
			      _header.num_of_jm,
			      _offset_num_mpr);
  TRY_HAS_FORTRAN_BLOCK_ITEMS(mr_antoine_istate_item_t,
			      _header.nsd,
			      _offset_istate);
  for (int i = 0; i < 2; i++)
    TRY_HAS_FORTRAN_BLOCK_ITEMS(mr_antoine_occ_item_t,
				_header.A[i] * _header.nslt[i],
				_offset_occ[i]);
  /*
    TRY_HAS_FORTRAN_BLOCK(mr_antoine_fon_ben_t,
    _offset_fon_ben);
  */
  
  return true;
}

template<class header_version_t>
bool mr_antoine_reader<header_version_t>::level2_read()
{
  ALLOC_GET_FORTRAN_BLOCK_ITEMS(_nr_ll_jj,
				_header.num_of_shell,
				_offset_nr_ll_jj);
  ALLOC_GET_FORTRAN_BLOCK_ITEMS(_num_mpr,
				_header.num_of_jm,
				_offset_num_mpr);
  /*
    These items are too large for level2_read!  We are not allowed
    to assume that they fit in memory.

    ALLOC_GET_FORTRAN_BLOCK_ITEMS(_istate,
				  _header.nsd,
				  _offset_istate);
    for (int i = 0; i < 2; i++)
      ALLOC_GET_FORTRAN_BLOCK_ITEMS(_occ[i],
				    _header.A[i] * _header.nslt[i],
				    _offset_occ[i]);
  */
  /*
    ALLOC_GET_FORTRAN_BLOCK_ITEMS(_fon_ben,1,
                                  _offset_fon_ben);
  */

  return true;
}

template<class header_version_t>
void mr_antoine_reader<header_version_t>::dump_info()
{
  printf ("===================================\n");
  printf ("== %sHeader%s ==\n",
	  CT_OUT(BOLD_BLUE),
	  CT_OUT(NORM_DEF_COL));
  
  printf ("num_of_shell ..: %s%d%s\n",
	  CT_OUT(BOLD_MAGENTA),
	  _header.num_of_shell,
	  CT_OUT(NORM_DEF_COL));
  printf ("num_of_jm .....: %s%d%s\n",
	  CT_OUT(BOLD_MAGENTA),
	  _header.num_of_jm,
	  CT_OUT(NORM_DEF_COL));
  printf ("nsd ...........: %s%d%s\n",
	  CT_OUT(BOLD_MAGENTA),
	  _header.nsd,
	  CT_OUT(NORM_DEF_COL));
  printf ("A[0,1] ........: %s%3d %3d%s\n",
	  CT_OUT(BOLD_MAGENTA),
	  _header.A[0],_header.A[1],
	  CT_OUT(NORM_DEF_COL));
  printf ("nslt[0,1] .....: %s%3d %3d%s\n",
	  CT_OUT(BOLD_MAGENTA),
	  _header.nslt[0],_header.nslt[1],
	  CT_OUT(NORM_DEF_COL));
  
  printf ("===================================\n");
  printf ("== %snr, ll, jj%s ==\n",
	  CT_OUT(BOLD_BLUE),
	  CT_OUT(NORM_DEF_COL));
  
  for (uint32_t i = 0; i < 10 && i < _header.num_of_shell-1; i++)
    printf ("#%s%3d%s: %s%3d %3d %3d%s\n",
	    CT_OUT(GREEN),
	    i+1,
	    CT_OUT(NORM_DEF_COL),
	    CT_OUT(MAGENTA),
	    _nr_ll_jj[i].nr,
	    _nr_ll_jj[i].ll,
	    _nr_ll_jj[i].jj,
	    CT_OUT(NORM_DEF_COL));
  
  if (_header.num_of_shell > 11)
    printf ("...\n");
  
  {
    uint32_t i = _header.num_of_shell-1;
    printf ("#%s%3d%s: %s%3d %3d %3d%s\n",
	    CT_OUT(GREEN),
	    i+1,
	    CT_OUT(NORM_DEF_COL),
	    CT_OUT(MAGENTA),
	    _nr_ll_jj[i].nr,
	    _nr_ll_jj[i].ll,
	    _nr_ll_jj[i].jj,
	    CT_OUT(NORM_DEF_COL));
  }
  
  uint32_t max_nr = 0;
  uint32_t max_ll = 0;
  uint32_t max_jj = 0;
  
  for (uint32_t i = 0; i < _header.num_of_shell; i++)
    {
      if (_nr_ll_jj[i].nr > max_nr)
	max_nr = _nr_ll_jj[i].nr;
      if (_nr_ll_jj[i].ll > max_ll)
	max_ll = _nr_ll_jj[i].ll;
      if (_nr_ll_jj[i].jj > max_jj)
	max_jj = _nr_ll_jj[i].jj;	
    }
  
  printf (" max: %s%3d %3d %3d%s\n",
	  CT_OUT(BOLD_MAGENTA),
	  max_nr, max_ll, max_jj,
	  CT_OUT(NORM_DEF_COL));
  
  printf ("===================================\n");
  printf ("== %snum, mpr%s ==\n",
	  CT_OUT(BOLD_BLUE),
	  CT_OUT(NORM_DEF_COL));
  
  for (uint32_t i = 0; i < 10 && i < _header.num_of_jm-1; i++)
    printf ("#%s%3d%s: %s%3d %3d%s\n",
	    CT_OUT(GREEN),
	    i+1,
	    CT_OUT(NORM_DEF_COL),
	    CT_OUT(MAGENTA),
	    _num_mpr[i].num,
	    _num_mpr[i].mpr,
	    CT_OUT(NORM_DEF_COL));
  
  if (_header.num_of_shell > 11)
    printf ("...\n");
  
  {
    uint32_t i = _header.num_of_jm-1;
    
    printf ("#%s%3d%s: %s%3d %3d%s\n",
	    CT_OUT(GREEN),
	    i+1,
	    CT_OUT(NORM_DEF_COL),
	    CT_OUT(MAGENTA),
	    _num_mpr[i].num,
	    _num_mpr[i].mpr,
	    CT_OUT(NORM_DEF_COL));
  }
  
  uint32_t max_num = 0;
  uint32_t max_mpr = 0;
  
  for (uint32_t i = 0; i < _header.num_of_jm; i++)
    {
      if (_num_mpr[i].num > max_num)
	max_num = _num_mpr[i].num;
      if (_num_mpr[i].mpr > max_mpr)
	max_mpr = _num_mpr[i].mpr;
    }
  
  printf (" max: %s%3d %3d%s\n",
	  CT_OUT(BOLD_MAGENTA),
	  max_num, max_mpr,
	  CT_OUT(NORM_DEF_COL));
  
  printf ("===================================\n");
  printf ("== %socc%s ==\n",
	  CT_OUT(BOLD_BLUE),
	  CT_OUT(NORM_DEF_COL));

  for (int k = 0; k < 2; k++)
    {
      unsigned int chunk = 10;
      if (chunk > _header.nslt[k] - 1)
	chunk = _header.nslt[k] - 1;

      dump_occ_chunk(k, 0, chunk);

      if (_header.nslt[k] > 11)
	printf ("...\n");

      dump_occ_chunk(k, _header.nslt[k] - 1, 1);

      if (k == 0)
	printf ("-----------------------------------\n");
    }

  printf ("===================================\n");
  printf ("== %sistate%s ==\n",
	  CT_OUT(BOLD_BLUE),
	  CT_OUT(NORM_DEF_COL));

  if (_config._dump == DUMP_FULL)
    {
      for (unsigned int start = 0; start < _header.nsd; )
	{
	  unsigned int num = 1000000;

	  if (num > _header.nsd - start)
	    num = _header.nsd - start;

	  dump_istate_chunk(start, num);

	  start += num;
	}
    }
  else
    {
      unsigned int chunk = 10;
      if (chunk > _header.nsd - 1)
	chunk = _header.nsd - 1;

      dump_istate_chunk(0, chunk);

      if (_header.nsd > 11)
	printf ("...\n");

      dump_istate_chunk(_header.nsd - 1, 1);
    }

  printf ("===================================\n");
}

template<class header_version_t>
void mr_antoine_reader<header_version_t>::
dump_occ_chunk(int k,uint32_t start,uint32_t num)
{
  mr_mapped_data h;

  void *data =
    MAP_BLOCK_DATA(_offset_occ[k] +
		   start * _header.A[k] * sizeof(uint32_t),
		   num * _header.A[k] * sizeof(uint32_t), h);

  mr_antoine_occ_item_t *pocc = (mr_antoine_occ_item_t *) data;

  for (unsigned int i = 0; i < num; i++)
    {
      printf ("#%s%3d%s:%s",
	      CT_OUT(GREEN),
	      start+i+1,
	      CT_OUT(NORM_DEF_COL),
	      CT_OUT(MAGENTA));

      for (unsigned int j = 0; j < _header.A[k]; j++)
	{
	  printf (" %3d", (pocc++)->sp);
	}

      printf ("%s\n",CT_OUT(NORM_DEF_COL));
    }

  h.unmap();
}

template<class header_version_t>
void mr_antoine_reader<header_version_t>::
dump_istate_chunk(uint32_t start,uint32_t num)
{
  mr_mapped_data h;

  void *data =
    MAP_BLOCK_DATA(_offset_istate +
		   start * sizeof(mr_antoine_istate_item_t),
		   num * sizeof(mr_antoine_istate_item_t), h);

  mr_antoine_istate_item_t *pistate = (mr_antoine_istate_item_t *) data;

  for (unsigned int i = 0; i < num; i++)
    {
      printf ("#%s%3d%s:%s",
	      CT_OUT(GREEN),
	      start+i+1,
	      CT_OUT(NORM_DEF_COL),
	      CT_OUT(MAGENTA));

      for (unsigned int j = 0; j < 2; j++)
	{
	  printf (" %3d", pistate->istate[j]);
	}
      pistate++;

      printf ("%s\n",CT_OUT(NORM_DEF_COL));
    }

  h.unmap();
}

template<class header_version_t>
void mr_antoine_reader<header_version_t>::find_used_states()
{
  /* First find out which occ states are used by the istates. */
  /*
  BITSONE_CONTAINER_TYPE      _occ_used[2];
#define BITSONE_CONTAINER_BITS    (sizeof(BITSONE_CONTAINER_TYPE)*8)
  */

  for (int i = 0; i < 2; i++)
    {
      size_t n = _header.nslt[i];

      _occ_used_items[i] =
	(n + BITSONE_CONTAINER_BITS-1) / BITSONE_CONTAINER_BITS;

      _occ_used[i] = (BITSONE_CONTAINER_TYPE *)
	malloc(_occ_used_items[i] * sizeof (BITSONE_CONTAINER_TYPE));

      if (!_occ_used[i])
	FATAL("Memory allocation failure (_occ_used).");

      memset(_occ_used[i], 0,
	     _occ_used_items[i] * sizeof (BITSONE_CONTAINER_TYPE));
    }

  for (unsigned int start = 0; start < _header.nsd; )
    {
      unsigned int num = 1000000;

      if (num > _header.nsd - start)
	num = _header.nsd - start;

      mr_mapped_data h;

      void *data =
	MAP_BLOCK_DATA(_offset_istate +
		       start * sizeof(mr_antoine_istate_item_t),
		       num * sizeof(mr_antoine_istate_item_t), h);

      mr_antoine_istate_item_t *pistate = (mr_antoine_istate_item_t *) data;

      for (unsigned int j = 0; j < num; j++)
	{
	  for (unsigned int i = 0; i < 2; i++)
	    {
	      uint32_t occ = pistate->istate[i];

	      BITSONE_CONTAINER_TYPE mask =
		((BITSONE_CONTAINER_TYPE) 1) <<
		(occ % (BITSONE_CONTAINER_BITS));
	      size_t offset = occ / (BITSONE_CONTAINER_BITS);

	      _occ_used[i][offset] |= mask;
	    }
	  pistate++;
	}

      h.unmap();

      start += num;
    }

  for (int i = 0; i < 2; i++)
    {
      long occ_used = 0;

      for (size_t j = 0; j < _occ_used_items[i]; j++)
	{
	  /*
	  printf ("%016lx %d\n",
		  _occ_used[i][j],
		  __builtin_popcountl(_occ_used[i][j]));
	  */
	  occ_used += __builtin_popcountl(_occ_used[i][j]);
	}

      printf ("OCC %d used: %ld (%zd)\n", i, occ_used, _occ_used_items[i]);
    }

  /* And then find out which single-particle states are used
   * by the occs.
   */

  _jm_used_slots = _header.A[0] + _header.A[1];
  _jm_used_items_per_slot =
    (_header.num_of_jm + BITSONE_CONTAINER_BITS-1) / BITSONE_CONTAINER_BITS;

  _jm_used = (BITSONE_CONTAINER_TYPE *)
    malloc (_jm_used_slots * _jm_used_items_per_slot *
	    sizeof (BITSONE_CONTAINER_TYPE));

  if (!_jm_used)
    FATAL("Memory allocation failure (_jm_used).");

  memset(_jm_used, 0,
	 _jm_used_slots * _jm_used_items_per_slot *
	 sizeof (BITSONE_CONTAINER_TYPE));

  BITSONE_CONTAINER_TYPE *jm_u = _jm_used;

  for (int i = 0; i < 2; i++)
    {
      for (unsigned int start = 0; start < _header.nslt[i]; )
	{
	  unsigned int num = 1000000;

	  if (num > _header.nslt[i] - start)
	    num = _header.nslt[i] - start;

	  mr_mapped_data h;

	  void *data =
	    MAP_BLOCK_DATA(_offset_occ[i] +
			   start * _header.A[i] * sizeof(uint32_t),
			   num * _header.A[i] * sizeof(uint32_t), h);

	  mr_antoine_occ_item_t *pocc = (mr_antoine_occ_item_t *) data;

	  for (unsigned int k = 0; k < num; k++)
	    {
	      BITSONE_CONTAINER_TYPE *jm_u_k = jm_u;

	      for (unsigned int j = 0; j < _header.A[i]; j++)
		{
		  uint32_t jm = (pocc++)->sp;

		  BITSONE_CONTAINER_TYPE mask =
		    ((BITSONE_CONTAINER_TYPE) 1) <<
		    (jm % (BITSONE_CONTAINER_BITS));
		  size_t offset = jm / (BITSONE_CONTAINER_BITS);
		  /*
		  printf ("%d %d %d : %zd %016lx : %zd %zd\n",
			  i, k, j,
			  offset, mask,
			  _jm_used_items_per_slot, _jm_used_slots);
		  fflush(stdout);
		  */
		  jm_u_k[offset] |= mask;

		  jm_u_k += _jm_used_items_per_slot;
		}
	    }

	  h.unmap();

	  start += num;
	}

      jm_u += _header.A[i] * _jm_used_items_per_slot;
    }

  jm_u = _jm_used;

  for (size_t i = 0; i < _jm_used_slots; i++)
    {
      BITSONE_CONTAINER_TYPE *jm_u_i = jm_u + i * _jm_used_items_per_slot;

      int jm_used = 0;

      for (size_t j = 0; j < _jm_used_items_per_slot; j++)
	{
	  jm_used += __builtin_popcountl(jm_u_i[j]);
	}

      printf ("JM %zd used: %d\n", i, jm_used);
    }


}

#define INSTANTIATE_ANTOINE(header_t)					\
  template mr_antoine_reader<header_t>::				\
  mr_antoine_reader(mr_file_reader *file_reader);			\
  template mr_antoine_reader<header_t>::~mr_antoine_reader();		\
  template const char *mr_antoine_reader<header_t>::get_format_name();	\
  template bool mr_antoine_reader<header_t>::level1_read();		\
  template bool mr_antoine_reader<header_t>::level2_read();		\
  template void mr_antoine_reader<header_t>::dump_info();		\
  template void mr_antoine_reader<header_t>::				\
  dump_occ_chunk(int k,uint32_t start,uint32_t num);			\
  template void mr_antoine_reader<header_t>::				\
  dump_istate_chunk(uint32_t start,uint32_t num);			\
  template void mr_antoine_reader<header_t>::find_used_states();	\
  ;

INSTANTIATE_ANTOINE(mr_antoine_header_old_t);
INSTANTIATE_ANTOINE(mr_antoine_header_new_t);
