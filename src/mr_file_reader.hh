
#ifndef __MR_FILE_READER_HH__
#define __MR_FILE_READER_HH__

#include <stdint.h>
#include <stdlib.h>

struct mr_mapped_data
{
public:
  mr_mapped_data()
  {
    _addr_base = NULL;
    _map_size = 0;
  }

public:
  void  *_addr_base;
  size_t _map_size;

public:
  void unmap();
};

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
  ssize_t has_fortran_block(uint64_t offset,ssize_t size);
  bool get_fortran_block(uint64_t offset,
			 void *block1,size_t size1,
			 void *block2 = NULL,size_t size2 = 0);

  void get_fortran_block_data(uint64_t offset_data,void *block,size_t size);

  void *map_block_data(uint64_t offset_data,size_t size,
		       mr_mapped_data &handle);
};

#endif/*__MR_FILE_READER_HH__*/
