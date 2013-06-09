
#ifndef __MAGIC_ANTOINE_READ_HH__
#define __MAGIC_ANTOINE_READ_HH__

#include "mr_base_reader.hh"

struct mr_antoine_header_new_t
{
  uint32_t num_of_shell;
  uint32_t num_of_jm;
  uint32_t nsd;
  uint32_t A0;
  uint32_t A[2];
  uint32_t nslt[2];
};

struct mr_antoine_header_old_t
{
  uint32_t num_of_shell;
  uint32_t num_of_jm;
  uint32_t nsd;
  uint32_t A[2];
  uint32_t nslt[2];
};

struct mr_antoine_nr_ll_jj_item_t // * num_of_shell
{
  uint32_t nr;
  uint32_t ll;
  uint32_t jj;
};

struct mr_antoine_num_mpr_item_t // * num_of_jm
{
  uint32_t num;
  uint32_t mpr;
};

struct mr_antoine_istate_item_t // * nsd
{
  uint32_t istate[2];
};

struct mr_antoine_occ_item_t // A[i] * nslt[i], i = 1,2
{
  uint32_t sp;
};

struct mr_antoine_fon_ben_t
{
  uint32_t fon[13];
  double   ben;  
};

extern int _debug;

#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

template<class header_version_t>
class mr_antoine_reader
  : public mr_base_reader
{
public:
  mr_antoine_reader(mr_file_reader *file_reader)
    : mr_base_reader(file_reader)
  {
  }

  virtual ~mr_antoine_reader()
  {

  }

public:
  uint64_t _offset_nr_ll_jj;
  uint64_t _offset_num_mpr;
  uint64_t _offset_istate;
  uint64_t _offset_occ[2];
  uint64_t _offset_fon_ben;

public:
  header_version_t _header;
    
  mr_antoine_nr_ll_jj_item_t *_nr_ll_jj;
  mr_antoine_num_mpr_item_t  *_num_mpr;
  mr_antoine_istate_item_t   *_istate;
  mr_antoine_occ_item_t      *_occ[2];
  mr_antoine_fon_ben_t       *_fon_ben;

public:
  virtual bool level1_read()
  {
    uint64_t cur_offset = 0;
    
    TRY_GET_FORTRAN_BLOCK(_header);
    
    CHECK_REASONABLE_RANGE(_header.num_of_shell,1,100);
    CHECK_REASONABLE_RANGE(_header.num_of_jm,1,1000);
    CHECK_REASONABLE_RANGE(_header.A[0],1,40);
    CHECK_REASONABLE_RANGE(_header.A[1],1,40);
    
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

  virtual bool level2_read()
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

  virtual const char *get_format_name() 
  { 
    if (sizeof(header_version_t) == sizeof(mr_antoine_header_old_t))
      return "ANTOINE_OLD";
    else
      return "ANTOINE_NEW";
  }

  virtual void dump_info()
  {
    printf ("===================================\n");
    printf ("== Header ==\n");
    printf ("num_of_shell ..: %d\n",_header.num_of_shell);
    printf ("num_of_jm .....: %d\n",_header.num_of_jm);
    printf ("nsd ...........: %d\n",_header.nsd);
    printf ("A[0,1] ........: %3d %3d\n",_header.A[0],_header.A[1]);
    printf ("nslt[0,1] .....: %3d %3d\n",_header.nslt[0],_header.nslt[1]);

    printf ("===================================\n");
    printf ("== nr, ll, jj ==\n");

    for (uint32_t i = 0; i < 10 && i < _header.num_of_shell; i++)
      printf ("#%3d: %3d %3d %3d\n",
	      i,
	      _nr_ll_jj[i].nr,
	      _nr_ll_jj[i].ll,
	      _nr_ll_jj[i].jj);

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

    printf ("...\n");
    printf (" max: %3d %3d %3d\n",
	    max_nr, max_ll, max_jj);

    printf ("===================================\n");
    printf ("== num, mpr ==\n");

    for (uint32_t i = 0; i < 10 && i < _header.num_of_jm; i++)
      printf ("#%3d: %3d %3d\n",
	      i,
	      _num_mpr[i].num,
	      _num_mpr[i].mpr);

    uint32_t max_num = 0;
    uint32_t max_mpr = 0;

    for (uint32_t i = 0; i < _header.num_of_jm; i++)
      {
	if (_num_mpr[i].num > max_num)
	  max_num = _num_mpr[i].num;
	if (_num_mpr[i].mpr > max_mpr)
	  max_mpr = _num_mpr[i].mpr;
      }

    printf ("...\n");
    printf (" max: %3d %3d\n",
	    max_num, max_mpr);

    printf ("===================================\n");
  }
};

#endif/*__MAGIC_ANTOINE_READ_HH__*/
