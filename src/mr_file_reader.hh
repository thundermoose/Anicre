
#ifndef __MR_FILE_READER_HH__
#define __MR_FILE_READER_HH__

#include <stdint.h>
#include <stdlib.h>

class mr_file_reader
{
public:
  mr_file_reader();
  ~mr_file_reader();

public:
  int _fd;
  const char *_filename;

public:
  void open(const char *filename);

public:
  bool has_fortran_block(uint64_t offset,size_t size);
  bool get_fortran_block(uint64_t offset,void *block,size_t size);

  void get_fortran_block_data(uint64_t offset_data,void *block,size_t size);

};

#endif/*__MR_FILE_READER_HH__*/
