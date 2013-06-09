#ifndef __MR_BASE_READER_HH__
#define __MR_BASE_READER_HH__

#include "error.hh"
#include "mr_file_reader.hh"

class mr_base_reader
{
public:
  mr_base_reader(mr_file_reader *file_reader);
  virtual ~mr_base_reader();

public:
  mr_file_reader *_file_reader; // We do *not* own, i.e. destroy

public:
  // Make a first preliminary file check, enough to distinguish
  // between formats
  virtual bool level1_read() = 0;
  // Locate all records, and make sure that file integrity is ok.  Do
  // not check contents of arrays that possibly are huge
  virtual bool level2_read() = 0;

public:
  // Return a string identifying the file format
  virtual const char *get_format_name() = 0;

public:
  // Dump information from the headers and data blocks.
  virtual void dump_info() = 0;

};

////////////////////////////////////////////////////////////////////////

#define TRY_GET_FORTRAN_BLOCK(block)					\
  do {									\
    if (!_file_reader->get_fortran_block(cur_offset,			\
					 &block,sizeof(block)))		\
      return false;							\
    if (_debug >= 1)							\
      INFO(" Got correctly sized block for '" #block			\
	   "' (%" PRIuPTR " bytes).",sizeof(block));			\
    cur_offset += sizeof(block) + 2 * sizeof(uint32_t);			\
  } while (0)

#define TRY_HAS_FORTRAN_BLOCK(block,offset)				\
  do {									\
    if (!_file_reader->has_fortran_block(cur_offset,			\
					 sizeof(block)))		\
      return false;							\
    if (_debug >= 1)							\
      INFO(" Got correctly sized block for '" #block			\
	   "' (%" PRIuPTR " bytes).",sizeof(block));			\
    offset = cur_offset + sizeof(uint32_t);				\
    cur_offset += sizeof(block) + 2 * sizeof(uint32_t);			\
  } while (0)

#define TRY_HAS_FORTRAN_BLOCK_ITEMS(block_item,nitems,offset)		\
  do {									\
    if (!_file_reader->has_fortran_block(cur_offset,			\
					 sizeof(block_item)*nitems))	\
      return false;							\
    if (_debug >= 1)							\
      INFO(" Got correctly sized block for %" PRIuPTR			\
	   " items of '" #block_item					\
	   "' (%" PRIuPTR " bytes).",					\
	   (size_t)nitems,sizeof(block_item)*nitems);			\
    offset = cur_offset + sizeof(uint32_t);				\
    cur_offset += sizeof(block_item) * nitems + 2 * sizeof(uint32_t);	\
  } while (0)

#define ALLOC_GET_FORTRAN_BLOCK_ITEMS(items,nitems,offset)		\
  do {									\
    size_t __allocsize = sizeof(items[0]) * nitems;			\
    items = (__typeof__(items)) malloc(__allocsize);			\
    if (!items) {							\
      FATAL(" Memory allocation failure: %" PRIuPTR " bytes "		\
	    "for %" PRIuPTR " items of '" #items "'.",			\
	    __allocsize,(size_t)nitems);				\
    }									\
    _file_reader->get_fortran_block_data(offset,items,__allocsize);	\
  } while (0)

#define CHECK_REASONABLE_RANGE(value,min,max)				\
  do {									\
    if ((value) < (min) || (value) > (max)) {				\
      if (_debug > 1)							\
        INFO(" Value '" #value "' (%d) outside "			\
	     "reasonable range [%d,%d].",				\
	     (value),(min),(max));					\
      return false;							\
    }									\
  } while (0) 

#endif/*__MR_BASE_READER_HH__*/
