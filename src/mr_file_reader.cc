
#include "error.hh"
#include "mr_file_reader.hh"

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

mr_file_reader::mr_file_reader()
{
  _fd = -1;
}

mr_file_reader::~mr_file_reader()
{
  close(_fd);
}

void mr_file_reader::open(const char *filename)
{
  _filename = filename;

  _fd = ::open(filename,O_RDONLY);

  if (_fd == -1)
    {
      perror("open");
      FATAL("Failure opening '%s'.",filename);
    }
}

bool mr_file_reader::has_fortran_block(uint64_t offset,
				       size_t size)
{
  uint32_t check;
  ssize_t n;

  if (lseek(_fd,offset,SEEK_SET) == -1)
    {
      perror("lseek");
      FATAL("Seek error.");
    }

  n = read(_fd,&check,sizeof(uint32_t));
  //printf ("%d %d %d\n",(int)n,check,(int)size);
  if (n == -1)
    {
      perror("read");
      FATAL("Read error.");
    }
  if (n < (ssize_t) sizeof(uint32_t))
    return false;
  if (check != size)
    {
      uint32_t check2;

      // If there is a block of size 'check' here (i.e. we find the end
      // marker), then print that info... (for debugging)

      if (lseek(_fd,offset+sizeof(uint32_t)+check,SEEK_SET) == -1)
	{
	  perror("lseek");
	  FATAL("Seek error.");
	}
      
      n = read(_fd,&check2,sizeof(uint32_t));
      //printf ("%d\n",(int)n);
      if (n == -1)
	{
	  perror("read");
	  FATAL("Read error.");
	}
      if (n < (ssize_t) sizeof(uint32_t))
	INFO(" At offset %" PRIuPTR", "
	     "no fortran block (head -> tail outside file).",
	     offset);
      else if (check2 != check)
	INFO(" At offset %" PRIuPTR", "
	     "no fortran block (head != tail).",
	     offset);
      else
	INFO(" At offset %" PRIuPTR", "
	     "possible fortran block (head == tail), size %" PRIuPTR", "
	     "(expected %" PRIuPTR").",
	     offset,(size_t) check,(size_t) size);

      return false;
    }

  if (lseek(_fd,offset+sizeof(uint32_t)+size,SEEK_SET) == -1)
    {
      perror("lseek");
      FATAL("Seek error.");
    }

  n = read(_fd,&check,sizeof(uint32_t));
  //printf ("%d\n",(int)n);
  if (n == -1)
    {
      perror("read");
      FATAL("Read error.");
    }
  if (n < (ssize_t) sizeof(uint32_t) || check != size)
    return false;

  return true;
}

void mr_file_reader::get_fortran_block_data(uint64_t offset_data,
					    void *block,size_t size)
{
  if (lseek(_fd,offset_data,SEEK_SET) == -1)
    {
      perror("lseek");
      FATAL("Seek error.");
    }

  ssize_t n;

  n = read(_fd,block,size);
  if (n == -1)
    {
      perror("read");
      FATAL("Read error.");
    }
  if (n != (ssize_t) size)
    {
      FATAL(" Incomplete read of %" PRIuPTR " bytes.",size);
    }
}

bool mr_file_reader::get_fortran_block(uint64_t offset,
				       void *block,size_t size)
{
  if (!has_fortran_block (offset,size))
    return false;

  // Since we got the block header and footer, the data should never
  // fail, (even if file format guess is wrong)

  get_fortran_block_data(offset+sizeof(uint32_t),block,size);

  return true;
}
