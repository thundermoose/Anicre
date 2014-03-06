
#include "util.h"

#include <unistd.h>
#include <stdio.h>

void full_read(int fd, void *buf, size_t count)
{
  size_t total = count;

  while (count)
    {
      ssize_t n = read (fd, buf, count);

      if (n == -1)
	{
	  perror("read");
	  exit(1);
	}
      if (n == 0)
	{
	  fprintf (stderr, "End-of-file reading %zd, missing %zd.\n",
		   total, count);
	  exit(1);
	}
      count -= (size_t) n;
      buf += n;
    }
}


void full_write(int fd, void *buf, size_t count)
{
  size_t total = count;

  while (count)
    {
      ssize_t n = write (fd, buf, count);

      if (n == -1)
	{
	  perror("write");
	  exit(1);
	}
      if (n == 0)
	{
	  fprintf (stderr, "Failure writing %zd, missing %zd.\n",
		   total, count);
	  exit(1);
	}
      count -= (size_t) n;
      buf += n;
    }
}

