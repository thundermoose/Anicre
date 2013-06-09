
#ifndef __MAGIC_MBSI_READ_HH__
#define __MAGIC_MBSI_READ_HH__

#include "mr_base_reader.hh"

struct mr_mbsi_sp_header_t
{
  uint32_t _maxcls;
  uint32_t _nosps;
};

struct mr_mbsi_sp_prt_shell_orb_item_t // * _maxcls
{
  uint32_t _num_prt;
  uint32_t _num_shellmin;
  uint32_t _num_shellmax;
  uint32_t _num_orbital;
  uint32_t _num_sp_states_cls;
  uint32_t _num_wds;
};

struct mr_mbsi_sp_spstates_item_t // * _nosps * _maxcls
{
  uint32_t _n;
  uint32_t _l;
  uint32_t _2s;
  uint32_t _2j;
  uint32_t _2m_j;
  uint32_t _2T;
  uint32_t _2T_z;
  uint32_t _N;
};

struct mr_mbsi_sp_partitioning_item_t
{
  uint32_t _numlevels;
  uint32_t _partitionsize;
};

#endif/*__MAGIC_MBSI_READ_HH__*/
