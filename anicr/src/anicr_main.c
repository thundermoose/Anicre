
#include "anicr_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int compare_mp_state(const void *p1, const void *p2)
{
  const int *s1 = (const int *) p1;
  const int *s2 = (const int *) p2;

  int i;

  for (i = 0; i < CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1; i++)
    {
      if (*s1 < *s2)
	return -1;
      if (*s1 > *s2)
	return 1;
    }

  return 0;
}

int main()
{
  size_t num_mp = CFG_NUM_MP_STATES;
  size_t num_sp = CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1;

  size_t mp_sz = sizeof (int) * num_sp * num_mp;

  int *mp = (int *) malloc (mp_sz);

  if (!mp)
    {
      fprintf (stderr, "Memory allocation error.\n");
      exit(1);
    }

  int fd = open ("states_all_orig.bin", O_RDONLY);

  if (fd == -1)
    {
      perror("open");
      exit(1);
    }

  ssize_t n = read (fd, mp, mp_sz);

  if (n != (ssize_t) mp_sz)
    {
      fprintf (stderr, "Failure reading %zd, got %zd.\n",
	       mp_sz, n);
      exit(1);
    }

  close (fd);

  printf ("Read %zd mp states.\n", num_mp);

  qsort (mp, num_mp, sizeof (int) * num_sp, compare_mp_state);

  printf ("Sorted %zd mp states.\n", num_mp);

  return 0;
}
