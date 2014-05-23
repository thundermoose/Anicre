
#include "antoine_vector_read.hh"
#include "colourtext.hh"

#include "mr_config.hh"

#include "mr_file_chunk.hh"

extern int _debug;

template<int dummy>
mr_antoine_vector_reader<dummy>::
mr_antoine_vector_reader(mr_file_reader *file_reader)
  : mr_base_reader(file_reader)
{
}

template<int dummy>
mr_antoine_vector_reader<dummy>::
~mr_antoine_vector_reader()
{
}

template<int dummy>
const char *mr_antoine_vector_reader<dummy>::get_format_name()
{ 
  return "ANTOINE_VECTOR";
}

template<int dummy>
bool mr_antoine_vector_reader<dummy>::level1_read()
{ 
  uint64_t cur_offset = 0;
  
  TRY_GET_FORTRAN_BLOCK(_header);




  return true;
}

template<int dummy>
bool mr_antoine_vector_reader<dummy>::level2_read()
{ 
  return false;
}

template<int dummy>
void mr_antoine_vector_reader<dummy>::dump_info()
{ 

}



#define INSTANTIATE_ANTOINE(dummy)					\
  template mr_antoine_vector_reader<dummy>::				\
  mr_antoine_vector_reader(mr_file_reader *file_reader);		\
  template mr_antoine_vector_reader<dummy>::~mr_antoine_vector_reader(); \
  template const char *mr_antoine_vector_reader<dummy>::		\
  get_format_name();							\
  template bool mr_antoine_vector_reader<dummy>::level1_read();		\
  template bool mr_antoine_vector_reader<dummy>::level2_read();		\
  template void mr_antoine_vector_reader<dummy>::dump_info();

INSTANTIATE_ANTOINE(1);
