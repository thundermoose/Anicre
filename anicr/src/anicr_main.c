
#include "anicr_config.h"
#include "create.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>

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

      s1++;
      s2++;
    }

  return 0;
}

int *_mp = NULL;

uint64_t _lookups = 0;
uint64_t _found = 0;

int main()
{
  size_t num_mp = CFG_NUM_MP_STATES;
  size_t num_sp = CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1;

  size_t mp_sz = sizeof (int) * num_sp * num_mp;

  _mp = (int *) malloc (mp_sz);

  if (!_mp)
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

  ssize_t n = read (fd, _mp, mp_sz);

  if (n != (ssize_t) mp_sz)
    {
      fprintf (stderr, "Failure reading %zd, got %zd.\n",
	       mp_sz, n);
      exit(1);
    }

  close (fd);

  printf ("Read %zd mp states.\n", num_mp);

  qsort (_mp, num_mp, sizeof (int) * num_sp, compare_mp_state);

  printf ("Sorted %zd mp states.\n", num_mp);

  ammend_tables();

  size_t i;

  int *mp = _mp;

  for (i = 0; i < num_mp; i++)
    {
      annihilate_states(mp + CFG_NUM_SP_STATES0, mp);

      mp += CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1;
      
      if (i % 1000 == 0)
	{
	  printf ("anicr %zd / %zd\r", i, num_mp);
	  fflush (stdout);
	}
    }

  printf ("Annihilated-created for %zd mp states.\n", num_mp);

  printf ("Found %"PRIu64"/%"PRIu64".\n", _found, _lookups);

  return 0;
}

int find_mp_state(int *lookfor)
{
  size_t num_mp = CFG_NUM_MP_STATES;
  size_t num_sp = CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1;

  void *found =
    bsearch (lookfor,
	     _mp, num_mp, sizeof (int) * num_sp, compare_mp_state);

  if (found)
    _found++;
  _lookups++;

  return found != NULL;
}
