
#ifndef __ANTOINE_READ_HH__
#define __ANTOINE_READ_HH__

#include "mr_base_reader.hh"
#include "mr_file_chunk.hh"
#include "antoine_struct.hh"

#include <vector>

#define BITSONE_CONTAINER_TYPE    unsigned long
#define BITSONE_CONTAINER_BITS    (sizeof(BITSONE_CONTAINER_TYPE)*8)

template<class fon_en_version_t>
class mr_antoine_reader_wavefcn
{
public:
  fon_en_version_t _fon_en;

public:
  std::vector<uint64_t> _offset_coeff;

public:
  bool level1_read();

};

template<class header_version_t, class fon_en_version_t>
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

  typedef mr_antoine_reader_wavefcn<fon_en_version_t> wavefcn_t;

  std::vector<wavefcn_t *> _wavefcns;

public:
  BITSONE_CONTAINER_TYPE     *_occ_used[2];
  size_t                      _occ_used_items[2];

  BITSONE_CONTAINER_TYPE     *_jm_used;
  size_t                      _jm_used_items_per_slot;
  size_t                      _jm_used_slots;

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

protected:
  void dump_occ_chunk(int k,uint32_t start,uint32_t num);
  void dump_istate_chunk(mr_file_chunk<mr_antoine_istate_item_t> &chunk);
};

#endif/*__ANTOINE_READ_HH__*/
