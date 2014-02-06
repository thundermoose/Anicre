#ifndef __FILE_OUTPUT_HH__
#define __FILE_OUTPUT_HH__

#include <stdio.h>

class file_output
{
public:
  file_output(const char *prefix, const char *name);
  ~file_output();

protected:
  char  *_filename;
  FILE  *_fid;

public:
  void fprintf(const char *format, ...)
    __attribute__ ((__format__ (__printf__, 2, 3)));
  void fwrite(void *ptr, size_t size, size_t nmemb);
};

#endif//FILE_OUTPUT
