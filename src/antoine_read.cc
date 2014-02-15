
#include "antoine_read.hh"
#include "colourtext.hh"

#include "mr_config.hh"

#include "mr_file_chunk.hh"

#include "sp_states.hh"
#include "missing_mpr.hh"

#include "pack_mp_state.hh"

#include "file_output.hh"

#include <string.h>
#include <limits.h>

#include <algorithm>

extern int _debug;

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

template<class header_version_t, class fon_version_t>
mr_antoine_reader<header_version_t, fon_version_t>::
mr_antoine_reader(mr_file_reader *file_reader)
  : mr_base_reader(file_reader)
{
  _nr_ll_jj = NULL;
  _num_mpr = NULL;
  for (int i = 0; i < 2; i++)
    _occ_used[i] = NULL;
  _jm_used = NULL;
}

template<class header_version_t, class fon_version_t>
mr_antoine_reader<header_version_t, fon_version_t>::~mr_antoine_reader()
{
  free(_nr_ll_jj);
  free(_num_mpr);
  for (int i = 0; i < 2; i++)
    free(_occ_used[i]);
  free(_jm_used);

  for (size_t i = 0; i < _wavefcns.size(); i++)
    delete _wavefcns[i];
}


template<class header_version_t, class fon_version_t>
const char *mr_antoine_reader<header_version_t,
			      fon_version_t>::get_format_name()
{ 
  if (sizeof(header_version_t) == sizeof(mr_antoine_header_old_t))
    return "ANTOINE_OLD";
  else
    return "ANTOINE_NEW";
}

#define ANTOINE_COEFF_CHUNK_SZ 1000000

template<typename item_t>
void pick_items(double *dest, size_t num, size_t dest_stride,
		mr_file_reader *file_reader,
		uint64_t src_file_offset,
		uint64_t src_item_offset)
{
  mr_mapped_data h;

  item_t *src = (item_t *)
    file_reader->map_block_data(src_file_offset + 
				src_item_offset * sizeof (item_t),
				num * sizeof (item_t), h);

  for (size_t i = 0; i < num; i++)
    {
      *dest = *src;
      src++;
      dest += dest_stride;
    }

  h.unmap();
}

template<class fon_version_t>
void mr_antoine_reader_wavefcn<fon_version_t>::
fill_coeff(double *dest,
	   mr_file_reader *file_reader,
	   size_t src_off, size_t num,
	   size_t stride, size_t val_off)
{
  dest += val_off;

  (void) src_off;

  for ( ; num; )
    {
      set_coeff_info::iterator itup;
      coeff_info find;

      find._start = src_off;
      itup = _offset_coeff.upper_bound(find);
      --itup;

      const coeff_info &ci = *itup;

      assert(ci._start <= src_off);

      size_t use = ci._len;
      if (use > num)
	use = num;
      
      if (_fon._.iprec == 1)
	{
	  ::pick_items<float>(dest, use, stride,
			      file_reader,
			      ci._offset, src_off - ci._start);
	}
      else /* 0 or 2 */
	{
	  ::pick_items<double>(dest, use, stride,
			       file_reader,
			       ci._offset, src_off - ci._start);
	}

      src_off += use;
      num -= use;
    }
}

template<class header_version_t, class fon_version_t>
bool mr_antoine_reader<header_version_t, fon_version_t>::
level1_read_wavefcn(wavefcn_t *wavefcn,
		    uint64_t &cur_offset, uint32_t nsd)
{
  TRY_GET_FORTRAN_2_BLOCK(wavefcn->_fon, wavefcn->_en);
  CHECK_REASONABLE_RANGE_0(wavefcn->_fon._.iprec,2);
  /* And then there are to be blocks with the coefficients. */
  uint32_t start = 0;
  for ( ; start < nsd; )
    {
      uint32_t block_elem = nsd - start;
      if (block_elem > ANTOINE_COEFF_CHUNK_SZ)
	block_elem = ANTOINE_COEFF_CHUNK_SZ;

      uint64_t offset;

      if (wavefcn->_fon._.iprec == 1)
	{
	  float coeff_f;
	  TRY_HAS_FORTRAN_BLOCK_ITEMS(coeff_f, block_elem, offset);
	}
      else /* 0 or 2 */
	{
	  double coeff_d;
	  TRY_HAS_FORTRAN_BLOCK_ITEMS(coeff_d, block_elem, offset);
	}

      coeff_info ci;

      ci._start  = start;
      ci._len    = block_elem;
      ci._offset = offset;

      wavefcn->_offset_coeff.insert(ci);

      start += block_elem;
    }
  return true;
}

template<class header_version_t, class fon_version_t>
bool mr_antoine_reader<header_version_t, fon_version_t>::level1_read()
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

  for ( ; ; ) {
    wavefcn_t *wavefcn = new wavefcn_t;
    uint64_t &wavefcn_cur_offset = cur_offset;

    if (level1_read_wavefcn(wavefcn, wavefcn_cur_offset, _header.nsd))
      {
	_wavefcns.push_back(wavefcn);
	cur_offset = wavefcn_cur_offset;
      }
    else
      {
	delete wavefcn;
	break;
      }
  }

  VERIFY_EOF;

  /*
  for ( ; ; ) {
    if (SKIP_POSSIBLE_FORTRAN_BLOCK == -1)
      break;
  }
  */
  return true;
}

template<class header_version_t, class fon_version_t>
bool mr_antoine_reader<header_version_t, fon_version_t>::level2_read()
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

template<class header_version_t, class fon_version_t>
void mr_antoine_reader<header_version_t, fon_version_t>::dump_info()
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

  uint32_t end_shell = _header.num_of_shell;

  if (_config._dump != DUMP_FULL)
    {
      end_shell--;
      if (end_shell > 10)
	end_shell = 10;
    }
  
  for (uint32_t i = 0; i < end_shell; i++)
    printf ("#%s%3d%s: %s%3d %3d %3d%s %3d\n",
	    CT_OUT(GREEN),
	    i+1,
	    CT_OUT(NORM_DEF_COL),
	    CT_OUT(MAGENTA),
	    _nr_ll_jj[i].nr,
	    _nr_ll_jj[i].ll,
	    _nr_ll_jj[i].jj,
	    CT_OUT(NORM_DEF_COL),
	    _nr_ll_jj[i].nr * 2 + _nr_ll_jj[i].ll);

  if (_header.num_of_shell > end_shell)
    {
      if (_header.num_of_shell > end_shell + 1)
	printf ("...\n");

      uint32_t i = _header.num_of_shell-1;
      printf ("#%s%3d%s: %s%3d %3d %3d%s %3d\n",
	      CT_OUT(GREEN),
	      i+1,
	      CT_OUT(NORM_DEF_COL),
	      CT_OUT(MAGENTA),
	      _nr_ll_jj[i].nr,
	      _nr_ll_jj[i].ll,
	      _nr_ll_jj[i].jj,
	      CT_OUT(NORM_DEF_COL),
	      _nr_ll_jj[i].nr * 2 + _nr_ll_jj[i].ll);
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
  
  uint32_t end_jm = _header.num_of_jm;

  if (_config._dump != DUMP_FULL)
    {
      end_jm--;
      if (end_jm > 10)
	end_jm = 10;
    }

  for (uint32_t i = 0; i < end_jm; i++)
    printf ("#%s%3d%s: %s%3d %3d%s\n",
	    CT_OUT(GREEN),
	    i+1,
	    CT_OUT(NORM_DEF_COL),
	    CT_OUT(MAGENTA),
	    _num_mpr[i].num,
	    _num_mpr[i].mpr,
	    CT_OUT(NORM_DEF_COL));

  if (_header.num_of_jm > end_jm)
    {
      if (_header.num_of_jm > end_jm + 1)
	printf ("...\n");
  
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
  int32_t  max_mpr = 0;
  int32_t  min_mpr = 0;
  
  for (uint32_t i = 0; i < _header.num_of_jm; i++)
    {
      if (_num_mpr[i].num > max_num)
	max_num = _num_mpr[i].num;
      if (_num_mpr[i].mpr > max_mpr)
	max_mpr = _num_mpr[i].mpr;
      if (_num_mpr[i].mpr < min_mpr)
	min_mpr = _num_mpr[i].mpr;
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

#define CHUNK_DUMP_SZ 1000000

  if (_config._dump == DUMP_FULL)
    {
      for (mr_file_chunk<mr_antoine_istate_item_t>
	     cm_istate(_header.nsd, CHUNK_DUMP_SZ);
	   cm_istate.map_next(_file_reader, _offset_istate); )
	dump_istate_chunk(cm_istate);
    }
  else
    {
      mr_file_chunk<mr_antoine_istate_item_t>
	cm_istate(_header.nsd, CHUNK_DUMP_SZ);

      cm_istate.map(_file_reader, _offset_istate,
		    0, std::min<unsigned int>(10,_header.nsd));

      dump_istate_chunk(cm_istate);

      if (_header.nsd > 11)
	printf ("...\n");

      if (_header.nsd > 10)
	{
	  cm_istate.map(_file_reader, _offset_istate,
			_header.nsd - 1, 1);

	  dump_istate_chunk(cm_istate);
	}
    }

  if (_wavefcns.size())
    {
      printf ("===================================\n");

      printf ("== %swavefcns %s ==\n",
	      CT_OUT(BOLD_BLUE),
	      CT_OUT(NORM_DEF_COL));

      printf ("      mjtot parity    jt2  coul iprec        en\n");
    }

  for (size_t i = 0; i < _wavefcns.size(); i++)
    {
      wavefcn_t *wavefcn = _wavefcns[i];

      printf ("#%s%3zd%s: %s%5d %6d %6d %5d %5d %9.4f%s\n",
	      CT_OUT(GREEN),
	      i+1,
	      CT_OUT(NORM_DEF_COL),
	      CT_OUT(MAGENTA),
	      wavefcn->_fon._.mjtotal,
	      wavefcn->_fon._.iparity,
	      wavefcn->_fon._.jt2,
	      wavefcn->_fon._.coul,
	      wavefcn->_fon._.iprec,
	      wavefcn->_en,
	      CT_OUT(NORM_DEF_COL));

    }

  printf ("===================================\n");
}

template<class header_version_t, class fon_version_t>
void mr_antoine_reader<header_version_t, fon_version_t>::
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

template<class header_version_t, class fon_version_t>
void mr_antoine_reader<header_version_t, fon_version_t>::
dump_istate_chunk(mr_file_chunk<mr_antoine_istate_item_t> &chunk)
{
  mr_antoine_istate_item_t *pistate = chunk.ptr();

  for (unsigned int i = 0; i < chunk.num(); i++)
    {
      printf ("#%s%3d%s:%s",
	      CT_OUT(GREEN),
	      chunk.start()+i+1,
	      CT_OUT(NORM_DEF_COL),
	      CT_OUT(MAGENTA));

      for (unsigned int j = 0; j < 2; j++)
	{
	  printf (" %3d", pistate->istate[j]);
	}
      pistate++;

      printf ("%s\n",CT_OUT(NORM_DEF_COL));
    }
}


template<class header_version_t, class fon_version_t>
void mr_antoine_reader<header_version_t, fon_version_t>::find_used_states()
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

#define CHUNK_FIND_SZ 1000000

  for (mr_file_chunk<mr_antoine_istate_item_t>
	 cm_istate(_header.nsd, CHUNK_FIND_SZ);
       cm_istate.map_next(_file_reader, _offset_istate); )
    {
      mr_antoine_istate_item_t *pistate = cm_istate.ptr();

      for (unsigned int j = 0; j < cm_istate.num(); j++)
	{
	  for (unsigned int i = 0; i < 2; i++)
	    {
	      uint32_t occ = pistate->istate[i] - 1;

	      BITSONE_CONTAINER_TYPE mask =
		((BITSONE_CONTAINER_TYPE) 1) <<
		(occ % (BITSONE_CONTAINER_BITS));
	      size_t offset = occ / (BITSONE_CONTAINER_BITS);

	      _occ_used[i][offset] |= mask;
	    }
	  pistate++;
	}
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

  _jm_used_slots = _header.A[0] + _header.A[1] + 1;
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

  BITSONE_CONTAINER_TYPE *jm_u_all = _jm_used;
  BITSONE_CONTAINER_TYPE *jm_u = _jm_used + _jm_used_items_per_slot;

  size_t max_jm_for_jm_sz = 2 * sizeof (uint32_t) * _header.num_of_jm;

  uint32_t *_max_jm_for_jm = (uint32_t *) malloc (max_jm_for_jm_sz);

  memset(_max_jm_for_jm, 0, max_jm_for_jm_sz);

  char *jm_jm_used = (char *) malloc (_header.num_of_jm * _header.num_of_jm);

  memset (jm_jm_used, 0, _header.num_of_jm * _header.num_of_jm);

  for (int i = 0; i < 2; i++)
    {
      uint32_t *max_jm_for_jm = _max_jm_for_jm + _header.num_of_jm * i;

      for (mr_file_chunk<mr_antoine_occ_item_t>
	     cm_occ(_header.nslt[i], CHUNK_FIND_SZ, _header.A[i]);
	   cm_occ.map_next(_file_reader, _offset_occ[i]); )
	{
	  mr_antoine_occ_item_t *pocc = cm_occ.ptr();

	  for (unsigned int k = 0; k < cm_occ.num(); k++)
	    {
	      BITSONE_CONTAINER_TYPE *jm_u_k = jm_u;

	      mr_antoine_occ_item_t *pocc1 = pocc;

	      uint32_t jm_array[32];

	      for (unsigned int j = 0; j < _header.A[i]; j++)
		{
		  uint32_t jm = (pocc++)->sp - 1;

		  jm_array[j] = jm;

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
		  jm_u_all[offset] |= mask;

		  jm_u_k[offset] |= mask;

		  jm_u_k += _jm_used_items_per_slot;
		}

	      uint32_t jm_max_plus1 = (pocc1 + _header.A[i] - 1)->sp;
	  
	      for (unsigned int j = 0; j < _header.A[i] - 1; j++)
                {
		  uint32_t jm = (pocc1++)->sp - 1;

		  if (jm_max_plus1 > max_jm_for_jm[jm])
		    max_jm_for_jm[jm] = jm_max_plus1;
		}

	      for (unsigned int j1 = 0; j1 < _header.A[i] - 1; j1++)
		{
		  for (unsigned int j2 = j1 + 1; j2 < _header.A[i]; j2++)
		    {
		      jm_jm_used[jm_array[j1] +
				 jm_array[j2] * _header.num_of_jm] = 1;
		    }
		}
	      /*
	      (void) pocc1;
	      (void) max_jm_for_jm;
	      */
	    }
	}

      jm_u += _header.A[i] * _jm_used_items_per_slot;
    }

  for (unsigned int i = 0; i < _header.num_of_jm; i++)
    {
      printf ("max_jm_for_jm %4d: %4d %4d\n",
	      i,
	      _max_jm_for_jm[i],
	      _max_jm_for_jm[i + _header.num_of_jm]);
    }

  uint64_t combs = 0;

  for (unsigned int j1 = 0; j1 < _header.num_of_jm; j1++)
    {
      for (unsigned int j2 = 0; j2 < _header.num_of_jm; j2++)
	{
	  combs += jm_jm_used[j1 + j2 * _header.num_of_jm];
	}
    }

  printf ("jm x jm used: %"PRIu64" /  %"PRIu64"\n",
	  combs, (uint64_t) _header.num_of_jm * (uint64_t) _header.num_of_jm);

  jm_u = _jm_used;

  int sp_used = -1;

  for (size_t i = 0; i < _jm_used_slots; i++)
    {
      BITSONE_CONTAINER_TYPE *jm_u_i = jm_u + i * _jm_used_items_per_slot;

      int jm_used = 0;

      for (size_t j = 0; j < _jm_used_items_per_slot; j++)
	{
	  jm_used += __builtin_popcountl(jm_u_i[j]);
	}

      printf ("JM %2zd used: %4d\n", i, jm_used);

      if (i == 0)
	sp_used = jm_used;
    }

  (void) sp_used;

  /* Fetch all the sp states that actually are in use.
   * No need to included unused ones in tables.
   */

  vect_sp_state sps;

  /* Mapping. */

  int *sps_map = (int *) malloc (sizeof (int) * _header.num_of_jm);

  if (!sps_map)
    FATAL("Memory allocation failure (sps_map).");

  for (uint32_t i = 0; i < _header.num_of_jm; i++)
    sps_map[i] = -1;

  for (size_t j = 0; j < _jm_used_items_per_slot; j++)
    {
      BITSONE_CONTAINER_TYPE used = jm_u[j];

      int off = 0;

      for ( ; used; )
	{
	  if (used & 1)
	    {
	      size_t i = j * sizeof (used) * 8 + off;

	      mr_antoine_num_mpr_item_t &mpr = _num_mpr[i];
	      uint32_t sh = mpr.num;
	      mr_antoine_nr_ll_jj_item_t &shell = _nr_ll_jj[sh-1];

	      sps_map[i] = (int) sps.size();

	      sps.push_back(sp_state(shell.nr, shell.ll, shell.jj, mpr.mpr));
	    }

	  used >>= 1;
	  off++;
	}
    }

  /* Now that we know who are used, we can for each sp location in the
   * mp state find which is the maximum index used.
   */

  int *_jm_max_spi =
    (int *) malloc (sizeof (int) * _jm_used_slots);

  for (size_t i = 0; i < _jm_used_slots; i++)
    {
      _jm_max_spi[i] = -1;

      BITSONE_CONTAINER_TYPE *jm_u_i = jm_u + i * _jm_used_items_per_slot;

      for (size_t j = 0; j < _jm_used_items_per_slot; j++)
        {
	  BITSONE_CONTAINER_TYPE used = *(jm_u_i++);
	  int off = 0;

	  for ( ; used; )
	    {
	      if (used & 1)
		{
		  int orig_sp = (int) (j * sizeof (used) * 8 + off);
		  int map_sp = sps_map[orig_sp];

		  if (map_sp > _jm_max_spi[i])
		    _jm_max_spi[i] = map_sp;
		}
	      used >>= 1;
	      off++;
	    }
	}
    }

#define BIT_PACK_T uint64_t

  pack_mp_state<BIT_PACK_T> bit_packing;

  bit_packing.setup_pack(_header.A[0], _header.A[1],
			 &_jm_max_spi[1], &_jm_max_spi[1 + _header.A[0]]);

  for (size_t i = 1; i < _jm_used_slots; i++)
    {
      printf ("JM %2zd max sp %4d, %2d bits @ %2d in word %d\n",
	      i, _jm_max_spi[i],
	      bit_packing._items[i-1]._bits,
	      bit_packing._items[i-1]._shift,
	      bit_packing._items[i-1]._word);
    }

  /* To calculate the maximum energy, we must map the istate in chunks,
   * and for each chunk, we must map the occ states for both particle
   * kinds in chunks too...
   */

  int min_N = INT_MAX, max_N = 0;
  int min_m = INT_MAX, max_m = 0;
  int min_pos_m = INT_MAX, max_pos_m = INT_MIN;
  int min_neg_m = INT_MAX, max_neg_m = INT_MIN; // redundant
  int has_parity[2] = { 0, 0 };

  /* Also dump all the states.  We need each multi-particle state
   * explicitly.
   */

#define CHUNK_SZ 1000000

  size_t istate_chunk_sz = _header.nsd;
  if (istate_chunk_sz > CHUNK_SZ)
    istate_chunk_sz = CHUNK_SZ;

  if (sizeof (BIT_PACK_T) != sizeof (double))
    {
      /* Alignment will be broken. */
      /* Also!: allocation calculations below will be botched. */
      FATAL("sizeof (BIT_PACK_T) = %zd != sizeof (double) = %zd",
	    sizeof (BIT_PACK_T), sizeof (double));
    }

  int n_wavefcns = _wavefcns.size() > 0 ? 1 : 0;

  size_t mp_states_stride = bit_packing._words + n_wavefcns;

  size_t mp_states_sz =
    sizeof (BIT_PACK_T) * istate_chunk_sz * mp_states_stride;

  BIT_PACK_T *mp_states = (BIT_PACK_T *) malloc (mp_states_sz);

  if (!mp_states)
    FATAL("Memory allocation error (mp_states, %zd bytes).", mp_states_sz);

  printf ("%zd %zd\n", mp_states_sz, mp_states_stride);

#define FILENAME_STATES "/states_all_orig.bin"

  file_output *out_states = NULL;

  if (_config._td_dir)
    {
      out_states = new file_output(_config._td_dir, FILENAME_STATES);
    }

  for (mr_file_chunk<mr_antoine_istate_item_t>
	 cm_istate(_header.nsd, CHUNK_SZ);
       cm_istate.map_next(_file_reader, _offset_istate); )
    {
      /* For debugging, fill with ones (= -1). */

      memset (mp_states, -1, mp_states_sz);

      for (mr_file_chunk<mr_antoine_occ_item_t>
	     cm_occ0(_header.nslt[0], CHUNK_SZ, _header.A[0]);
	   cm_occ0.map_next(_file_reader, _offset_occ[0]); )
	{
	  mr_antoine_occ_item_t *pocc0 = cm_occ0.ptr();

	  for (mr_file_chunk<mr_antoine_occ_item_t>
		 cm_occ1(_header.nslt[1], CHUNK_SZ, _header.A[1]);
	       cm_occ1.map_next(_file_reader, _offset_occ[1]); )
	    {
	      mr_antoine_occ_item_t *pocc1 = cm_occ1.ptr();

	      mr_antoine_istate_item_t *pistate = cm_istate.ptr();

	      BIT_PACK_T *mp_ptr = mp_states;

	      for (unsigned int j = 0; j < cm_istate.num(); j++)
		{
		  if (pistate->istate[0]-1 >= cm_occ0.start() &&
		      pistate->istate[0]-1 < (cm_occ0.start() +
					      cm_occ0.num()) &&
		      pistate->istate[1]-1 >= cm_occ1.start() &&
		      pistate->istate[1]-1 < (cm_occ1.start() +
					      cm_occ1.num()))
		    {
		      int sum_N = 0;
		      int sum_m = 0;
		      int sum_pos_m = 0;
		      int sum_neg_m = 0;
		      int sum_l = 0;

		      mr_antoine_occ_item_t *poccs[2] =
			{
			  pocc0 + ((pistate->istate[0]-1 - cm_occ0.start()) *
				   _header.A[0]),
			  pocc1 + ((pistate->istate[1]-1 - cm_occ1.start()) *
				   _header.A[1]),
			};

		      BIT_PACK_T *mp_ptr_this = mp_ptr;

		      bit_packing.clear_packed(mp_ptr_this);

		      int kk = 0;

		      for (unsigned int i = 0; i < 2; i++)
			{
			  // uint32_t occ = pistate->istate[i];

			  mr_antoine_occ_item_t *pocc = poccs[i];

			  for (unsigned int k = 0; k < _header.A[i]; k++)
			    {
			      uint32_t jm = (pocc++)->sp - 1;

			      uint32_t sh = _num_mpr[jm].num;
			      int32_t  mpr = _num_mpr[jm].mpr;

			      mr_antoine_nr_ll_jj_item_t &shell =
				_nr_ll_jj[sh-1];

			      int N = 2 * shell.nr + shell.ll;

			      sum_N += N;
			      sum_m += mpr;
			      sum_l += shell.ll;
			      /*
			      printf ("%3d  (%2d %2d) %3d %3d  %3d %3d\n",
				      jm, shell.nr, shell.ll,
				      sum_m, sum_N,
				      mpr, N);
			      */

			      if (mpr > 0)
				sum_pos_m += mpr;
			      else
				sum_neg_m += mpr;

			      bit_packing.insert_packed(mp_ptr_this, kk++,
							sps_map[jm]);
			    }
			}
		      /*
		      printf ("N=%2d m=%3d\n", sum_N, sum_m);
		      */

		      if (sum_N > max_N)
			max_N = sum_N;
		      if (sum_N < min_N)
			min_N = sum_N;


		      if (sum_m > max_m)
			max_m = sum_m;
		      if (sum_m < min_m)
			min_m = sum_m;

		      if (sum_pos_m > max_pos_m)
			max_pos_m = sum_pos_m;
		      if (sum_pos_m < min_pos_m)
			min_pos_m = sum_pos_m;

		      if (sum_neg_m > max_neg_m)
			max_neg_m = sum_neg_m;
		      if (sum_neg_m < min_neg_m)
			min_neg_m = sum_neg_m;

		      has_parity[sum_l & 1] = 1;
		    }

		  pistate++;
		  mp_ptr += mp_states_stride;
		}
	    }
	}

      if (out_states)
	{
	  /* Fill in the wavefunctions. */

	  for (int i = 0; i < n_wavefcns; i++)
	    {
	      _wavefcns[i]->fill_coeff((double *) mp_states,
				       _file_reader,
				       cm_istate.start(), cm_istate.num(),
				       mp_states_stride,
				       bit_packing._words + i);
	    }

	  size_t mp_used_sz =
	    sizeof (BIT_PACK_T) * cm_istate.num() * mp_states_stride;

	  out_states->fwrite (mp_states, mp_used_sz, 1);
	}
    }

  if (out_states)
    delete out_states;

  free(mp_states);

  printf ("N_min:     %2d  N_max:     %2d\n", min_N, max_N);
  printf ("m_min:     %2d  m_max:     %2d\n", min_m, max_m);
  printf ("pos_m_min: %2d  pos_m_max: %2d\n", min_pos_m, max_pos_m);
  printf ("neg_m_min: %2d  neg_m_max: %2d\n", min_neg_m, max_neg_m);
  printf ("parity:    even %d odd %d\n", has_parity[0], has_parity[1]);

  if (min_m != max_m)
    FATAL("m is not the same for all states, range: [%d,%d].",
	  min_m, max_m);

  if (has_parity[0] && has_parity[1])
    FATAL("parity is not the same for all states (both even and odd).");

  int M = min_m;
  int parity = has_parity[1];

  ///////////////////////////////////////////////////////////////////////
  //
  // TODO: move following into some other function (logically separate)
  //
  ///////////////////////////////////////////////////////////////////////

  if (_config._td_dir)
    {
#define FILENAME_TABLE "/tables.h"

      file_output out_table(_config._td_dir, FILENAME_TABLE);

      out_table.fprintf("/* This file is automatically generated. */\n");
      out_table.fprintf("/* Editing is useless.                   */\n");
      out_table.fprintf("\n");

      bit_packing.generate_tables(out_table);

      sp_states_table(out_table, sps);

      missing_mpr_tables(out_table, M, parity, sps);
    }

  if (_config._td_dir)
    {
#define FILENAME_CONFIG "/config.h"

      file_output out_config(_config._td_dir, FILENAME_CONFIG);

      out_config.fprintf("/* This file is automatically generated. */\n");
      out_config.fprintf("/* Editing is useless.                   */\n");
      out_config.fprintf("\n");

      out_config.fprintf("#define CFG_NUM_MP_STATES   %d\n",
			 _header.nsd);
      out_config.fprintf("#define CFG_NUM_SP_STATES   %zd\n",
			 sps.size());
      out_config.fprintf("#define CFG_NUM_SP_STATES0  %d\n",
			 _header.A[0]);
      out_config.fprintf("#define CFG_NUM_SP_STATES1  %d\n",
			 _header.A[1]);
      out_config.fprintf("#define CFG_MAX_SUM_E       %d\n",
			 max_N);
      out_config.fprintf("#define CFG_SUM_M           %d\n",
			 max_m); /* = min_m */
      out_config.fprintf("#define CFG_PACK_WORDS      %d\n",
			 bit_packing._words);
      out_config.fprintf("#define CFG_WAVEFCNS        %d\n",
			 n_wavefcns);
    }

  if (_config._td_dir)
    {
      #define FILENAME_CODE "/code.h"

      file_output out_code(_config._td_dir, FILENAME_CODE);

      out_code.fprintf("/* This file is automatically generated. */\n");
      out_code.fprintf("/* Editing is useless.                   */\n");
      out_code.fprintf("\n");

      bit_packing.generate_code(out_code);

    }
}

#define INSTANTIATE_ANTOINE(header_t,fon_t)				\
  template mr_antoine_reader<header_t,fon_t>::				\
  mr_antoine_reader(mr_file_reader *file_reader);			\
  template mr_antoine_reader<header_t,fon_t>::~mr_antoine_reader();	\
  template const char *mr_antoine_reader<header_t,fon_t>::		\
  get_format_name();							\
  template bool mr_antoine_reader<header_t,fon_t>::level1_read();	\
  template bool mr_antoine_reader<header_t,fon_t>::level2_read();	\
  template void mr_antoine_reader<header_t,fon_t>::dump_info();		\
  template void mr_antoine_reader<header_t,fon_t>::			\
  dump_occ_chunk(int k,uint32_t start,uint32_t num);			\
  template void mr_antoine_reader<header_t,fon_t>::			\
  dump_istate_chunk(mr_file_chunk<mr_antoine_istate_item_t> &chunk);	\
  template void mr_antoine_reader<header_t,fon_t>::			\
  find_used_states();							\
  ;

INSTANTIATE_ANTOINE(mr_antoine_header_old_t,mr_antoine_fon_old_t);
INSTANTIATE_ANTOINE(mr_antoine_header_new_t,mr_antoine_fon_new_t);
