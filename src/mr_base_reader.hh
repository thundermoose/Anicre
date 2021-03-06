#ifndef __MR_BASE_READER_HH__
#define __MR_BASE_READER_HH__

#include "error.hh"
#include "mr_file_reader.hh"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

class mp_state_info;

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

public:
  // Figure out which sp and mp (partial) states are actually used.
  virtual void find_used_states() = 0;

public:
  // Info in mp state actually to be used, as initial or final.
  virtual void find_inifin_states(mp_state_info &mp_info) = 0;

public:
  // Generate the tables and code.
  virtual void create_code_tables(mp_state_info &mp_info) = 0;
};

////////////////////////////////////////////////////////////////////////

#define SKIP_POSSIBLE_FORTRAN_BLOCK					\
  (__extension__							\
   ({ ssize_t len = _file_reader->has_fortran_block(cur_offset,-1);	\
     if (len != -1)							\
       cur_offset += len + 2 * sizeof(uint32_t);			\
     len; }))

#define VERIFY_EOF					\
  do {							\
    if (!_file_reader->verify_eof(cur_offset))		\
      return false;					\
  } while (0)

#define TRY_GET_FORTRAN_BLOCK(block)					\
  do {									\
    if (!_file_reader->get_fortran_block(cur_offset,			\
					 &block,sizeof(block)))		\
      return false;							\
    if (_debug >= 1)							\
      INFO(" Good block for '%s' "					\
	   "(%" PRIuPTR " bytes).", #block, sizeof(block));		\
    cur_offset += sizeof(block) + 2 * sizeof(uint32_t);			\
  } while (0)

#define TRY_GET_FORTRAN_2_BLOCK(block1, block2)				\
  do {									\
    if (!_file_reader->get_fortran_block(cur_offset,			\
					 &block1,sizeof(block1),	\
					 &block2,sizeof(block2)))	\
      return false;							\
    if (_debug >= 1)							\
      INFO(" Good block for '%s'+'%s' "					\
	   "(%" PRIuPTR "+%" PRIuPTR " bytes).",			\
	   #block1, #block2, sizeof(block1), sizeof(block2));		\
    cur_offset +=							\
      sizeof(block1) + sizeof(block2) + 2 * sizeof(uint32_t);		\
  } while (0)

#define TRY_GET_FORTRAN_BLOCK_EXTRA(block, extra, extra_offset)		\
  do {									\
    ssize_t len = _file_reader->has_fortran_block(cur_offset,-1);	\
    if (len < (ssize_t) sizeof (block))					\
      return false;							\
    uint64_t __offset = cur_offset + sizeof(uint32_t);			\
    _file_reader->get_fortran_block_data(__offset,&block,sizeof(block)); \
    extra_offset = __offset + sizeof(block);				\
    extra = len - sizeof(block);					\
    if (_debug >= 1)							\
      INFO(" Good block for '%s'+extra "				\
	   "(%" PRIuPTR "+%" PRIuPTR " bytes).",			\
	   #block, sizeof(block), extra);				\
    cur_offset += len + 2 * sizeof(uint32_t);				\
  } while (0)

#define TRY_HAS_FORTRAN_BLOCK(block,offset)				\
  do {									\
    if (_file_reader->has_fortran_block(cur_offset,			\
					sizeof(block)) == -1)		\
      return false;							\
    if (_debug >= 1)							\
      INFO(" Good block for '%s' "					\
	   "(%" PRIuPTR " bytes).", #block, sizeof(block));		\
    offset = cur_offset + sizeof(uint32_t);				\
    cur_offset += sizeof(block) + 2 * sizeof(uint32_t);			\
  } while (0)

#define TRY_HAS_FORTRAN_BLOCK_ITEMS(block_item,nitems,offset)		\
  do {									\
    if (_file_reader->has_fortran_block(cur_offset,			\
					sizeof(block_item)*nitems) == -1) \
      return false;							\
    if (_debug >= 1)							\
      INFO(" Good block for %" PRIuPTR					\
	   " items of '%s' "						\
	   "(%" PRIuPTR " bytes).",					\
	   (size_t)nitems, #block_item, sizeof(block_item)*nitems);	\
    offset = cur_offset + sizeof(uint32_t);				\
    cur_offset += sizeof(block_item) * nitems + 2 * sizeof(uint32_t);	\
  } while (0)

#define ALLOC_GET_FORTRAN_BLOCK_ITEMS(items,nitems,offset)		\
  do {									\
    size_t __allocsize = sizeof(items[0]) * nitems;			\
    items = (__typeof__(items)) malloc(__allocsize);			\
    if (!items) {							\
      FATAL(" Memory allocation failure: %" PRIuPTR " bytes "		\
	    "for %" PRIuPTR " items of '%s'.",				\
	    __allocsize, (size_t)nitems, #items);			\
    }									\
    _file_reader->get_fortran_block_data(offset,items,__allocsize);	\
  } while (0)

#define MAP_BLOCK_DATA(offset,length,handle)		\
  _file_reader->map_block_data(offset,length,handle)

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

/* Checking a range that starts at 0 for an unsigned gives warning above */
#define CHECK_REASONABLE_RANGE_0(value,max)				\
  do {									\
    if ((value) > (max)) {						\
      if (_debug > 1)							\
        INFO(" Value '" #value "' (%d) outside "			\
	     "reasonable range [%d,%d].",				\
	     (value),(0),(max));					\
      return false;							\
    }									\
  } while (0)

#endif/*__MR_BASE_READER_HH__*/
