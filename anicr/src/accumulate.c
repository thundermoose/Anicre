
#include "accumulate.h"
#include "util.h"

#include "anicr_tables.h"
#include "anicr_config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "tmp_config.h"

uint32_t *_jm_pairs = NULL;

void prepare_accumulate()
{
  /* Read in the jm pairs info. */

  size_t sz_jm_pairs = sizeof (uint32_t) * CFG_JM_PAIRS;

  _jm_pairs = (uint32_t*) malloc (sz_jm_pairs);

  int fd = open ("states_all_orig.bin", O_RDONLY);

  if (fd == -1)
    {
      perror("open");
      exit(1);
    }

  full_read (fd, _jm_pairs, sz_jm_pairs);

  close (fd);

  printf ("Read %zd jm pairs states.\n", (size_t) CFG_JM_PAIRS);
}

double *_accumulate;

void alloc_accumulate()
{
  size_t num_accum;

#if ANICR2
  num_accum = CFG_TOT_FIRST_SCND * CFG_TOT_FIRST_SCND;
#else
  num_accum = CFG_NUM_SP_STATES * CFG_NUM_SP_STATES;
#endif

  size_t accum_sz = sizeof (double) * num_accum;

  _accumulate = (double *) malloc (accum_sz);

  if (!_accumulate)
    {
      fprintf (stderr, "Memory allocation error (%zd bytes).\n", accum_sz);
      exit(1);
    }

  printf ("Allocated %zd items for accumulation.\n", num_accum);

  memset (_accumulate, 0, accum_sz); 
}

