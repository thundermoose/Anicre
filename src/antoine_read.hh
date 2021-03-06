
#ifndef __ANTOINE_READ_HH__
#define __ANTOINE_READ_HH__

#include "mr_base_reader.hh"
#include "mr_file_chunk.hh"
#include "antoine_struct.hh"

#include "sp_states.hh"
#include "pack_mp_state.hh"

#include "mp_state_info.hh"

#include "sp_pair_use.hh"

#include <set>
#include <vector>

struct coeff_info
{
  uint64_t _start;   /* first item. */
  uint64_t _len;     /* number of items */
  uint64_t _offset;  /* block offset in file */  

public:
  bool operator<(const struct coeff_info &rhs) const {
    return _start < rhs._start;
  }
};

template<class fon_version_t>
class mr_antoine_reader_wavefcn
{
public:
  fon_version_t _fon;
  double _en;

public:
  typedef std::set<coeff_info> set_coeff_info;

  set_coeff_info _offset_coeff;

public:
  bool level1_read();

public:
  void fill_coeff(double *dest,
		  mr_file_reader *file_reader,
		  size_t src_off, size_t num,
		  size_t stride, size_t val_off);

  void dump_coeff(mr_file_reader *file_reader,
		  size_t src_off, size_t num);

};

template<class header_version_t, class fon_version_t>
class mr_antoine_reader
  : public mr_base_reader
{
public:
  mr_antoine_reader(mr_file_reader *file_reader);
  virtual ~mr_antoine_reader();

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
  //mr_antoine_istate_item_t   *_istate;
  //mr_antoine_occ_item_t      *_occ[2];
  //mr_antoine_fon_ben_t       *_fon_ben;

  typedef mr_antoine_reader_wavefcn<fon_version_t> wavefcn_t;

  std::vector<wavefcn_t *> _wavefcns;

public:
  BITSONE_CONTAINER_TYPE     *_occ_used[2];
  size_t                      _occ_used_items[2];

  BITSONE_CONTAINER_TYPE     *_jm_used;
  size_t                      _jm_used_items_per_slot;
  size_t                      _jm_used_slots;

  uint32_t *_max_jm_for_jm;  // TO BE REMOVED

  BITSONE_CONTAINER_TYPE     *_nlj_used;
  size_t                      _nlj_used_items_per_slot;

  vect_nlj_state              _nljs;
  int                        *_nljs_map;

  uint32_t                    _max_j;

  uint32_t                    _nhomax;

  vect_sp_state               _sps;
  int                        *_sps_map;

#define BIT_PACK_T uint64_t

  pack_mp_state<BIT_PACK_T>   _bit_packing[2];

  int                         _n_wavefcns;

  sp_pair_use                 _mapped_sp_pair_use[3]; // 00, 11, 01

public:
  bool level1_read_wavefcn(wavefcn_t *wavefcn,
			   uint64_t &cur_offset, uint32_t nsd);

public:
  virtual bool level1_read();
  virtual bool level2_read();

public:
  virtual const char *get_format_name();

public:
  virtual void dump_info();

public:
  virtual void find_used_states();

  virtual void find_inifin_states(mp_state_info &mp_info);

  virtual void create_code_tables(mp_state_info &mp_info);

protected:
  void find_occ_used();
  void find_jm_used();
  void info_jm_used();
  void find_nlj_used();
  void make_nlj_map();
  void make_sps_map();
  void find_sp_pairs();
  void find_mp_bit_packing();
  void find_energy_dump_states(mp_state_info &mp_info);
  void dump_wavefcn();

protected:
  void dump_occ_chunk(int k,uint32_t start,uint32_t num);
  void dump_istate_chunk(mr_file_chunk<mr_antoine_istate_item_t> &chunk);
};

#endif/*__ANTOINE_READ_HH__*/
