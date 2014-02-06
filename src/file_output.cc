
#include "file_output.hh"
#include "error.hh"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

file_output::file_output(const char *prefix, const char *name)
{
  _filename =
    (char *) malloc(strlen(prefix) + strlen(name) + 1);
  
  if (!_filename)
    ERROR("Memory allocation error (_filename).");
  
  strcpy(_filename, prefix);
  strcat(_filename, name);
  
  if ((_fid = fopen(_filename, "w")) == NULL)
    {
      perror("fopen");
      ERROR("Failed to open '%s' for writing.", _filename);
    }
}

file_output::~file_output()
{
  if (ferror(_fid))
    {
      ERROR("There has been an error during writing to '%s'.",
	    _filename);
    }
  
  if (fclose(_fid) != 0)
    {
      perror("fclose");
      ERROR("Failed to close '%s' after writing.", _filename);
    }
  
  free(_filename);
}

void file_output::fprintf(const char *format, ...)
{
  va_list ap;

  va_start(ap,format);
  int n = vfprintf (_fid, format, ap);
  va_end(ap);

  if (n < 0)
    ERROR("Errro writing to '%s'.", _filename);
}

void file_output::fwrite(void *ptr, size_t size, size_t nmemb)
{
  size_t ret = ::fwrite(ptr, size, nmemb, _fid);

  if (ret != nmemb)
    ERROR("Errro writing to '%s'.", _filename);
}

