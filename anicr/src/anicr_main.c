
#include "anicr_config.h"
#include "create.h"
#include "packed_create.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

int compare_packed_mp_state(const void *p1, const void *p2)
{
  const uint64_t *s1 = (const uint64_t *) p1;
  const uint64_t *s2 = (const uint64_t *) p2;

  int i;

  for (i = 0; i < CFG_PACK_WORDS; i++)
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

uint64_t *_mp = NULL;

uint64_t _lookups = 0;
uint64_t _found = 0;

int main(int argc, char *argv[])
{
  size_t num_mp = CFG_NUM_MP_STATES;
  /* size_t num_sp = CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1; */

  size_t mp_sz = sizeof (uint64_t) * CFG_PACK_WORDS * num_mp;

  _mp = (uint64_t *) malloc (mp_sz);

  if (!_mp)
    {
      fprintf (stderr, "Memory allocation error (%zd bytes).\n", mp_sz);
      exit(1);
    }

  int fd = open ("states_all_orig.bin", O_RDONLY);

  if (fd == -1)
    {
      perror("open");
      exit(1);
    }

  size_t toread = mp_sz;
  void *dest = _mp;

  while (toread)
    {
      ssize_t n = read (fd, dest, toread);

      if (n == -1)
	{
	  perror("read");
	  exit(1);
	}
      if (n == 0)
	{
	  fprintf (stderr, "End-of-file reading %zd, missing %zd.\n",
		   mp_sz, toread);
	  exit(1);
	}
      toread -= (size_t) n;
      dest += n;
    }

  close (fd);

  printf ("Read %zd mp states.\n", num_mp);

  qsort (_mp, num_mp, sizeof (uint64_t) * CFG_PACK_WORDS,
	 compare_packed_mp_state);

  printf ("Sorted %zd mp states.\n", num_mp);

  ammend_tables();

  size_t i;

  uint64_t *mp = _mp;

  for (i = 0; i < num_mp; i++)
    {
	/* annihilate_states(mp + CFG_NUM_SP_STATES0, mp); */
	annihilate_packed_states(mp);

      mp += CFG_PACK_WORDS;
      
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

int find_mp_state(uint64_t *lookfor)
{
  size_t num_mp = CFG_NUM_MP_STATES;
  /* size_t num_sp = CFG_NUM_SP_STATES0 + CFG_NUM_SP_STATES1; */

  void *found =
    bsearch (lookfor,
	     _mp, num_mp, sizeof (uint64_t) * CFG_PACK_WORDS,
	     compare_packed_mp_state);

  if (found)
    _found++;
  _lookups++;

  return found != NULL;
}
