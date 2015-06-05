
#ifndef __ANTOINE_VECTOR_READ_HH__
#define __ANTOINE_VECTOR_READ_HH__

#include "mr_base_reader.hh"
#include "mr_file_chunk.hh"

#include "antoine_vector_struct.hh"

template<int dummy>
class mr_antoine_vector_reader
  : public mr_base_reader
{
public:
  mr_antoine_vector_reader(mr_file_reader *file_reader);
  virtual ~mr_antoine_vector_reader();

public:
  mr_antoine_vector_header_t _header;

  mr_antoine_vector_info1_t  _info1;

  uint64_t _offset_cut;

public:
  virtual bool level1_read();
  virtual bool level2_read();

public:
  virtual const char *get_format_name();

public:
  virtual void dump_info();

public:
  virtual void find_used_states() { }
  virtual void find_inifin_states(mp_state_info &/*mp_info*/) { }
  virtual void create_code_tables(mp_state_info &/*mp_info*/) { }
};

#endif/*__ANTOINE_VECTOR_READ_HH__*/
