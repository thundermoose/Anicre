#include "sp_pair_use.hh"
#include "error.hh"

#include <string.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

sp_pair_use::sp_pair_use()
{
  _used = NULL;
  _n1 = _n2 = _sz_line_n2 = 0;
  _num_pairs = 0;
}

sp_pair_use::~sp_pair_use()
{
  free (_used);
}

void sp_pair_use::alloc(size_t n1, size_t n2)
{
  size_t sz_line_n2 =
    (n2 + BITSONE_CONTAINER_BITS-1) / BITSONE_CONTAINER_BITS;

  size_t sz_line_n1 =
    n1 * sz_line_n2;

  size_t sz_used = sizeof (BITSONE_CONTAINER_TYPE) * sz_line_n1;

  _used = (BITSONE_CONTAINER_TYPE *) malloc (sz_used);

  if (!_used)
    FATAL("Memory allocation error (sp_pair_use::_used, %zd bytes).",
	  sz_used);

  memset(_used, 0, sz_used);

  _n1 = n1;
  _n2 = n2;
  _sz_line_n2 = sz_line_n2;

  printf ("sp used  %zd %zd %zd\n", _n1, _n2, _sz_line_n2);
}

void sp_pair_use::dump_pairs_used(file_output &out)
{
  uint64_t num_used = 0;

  /* Find the number of pairs in use. */

  for (size_t i = 0; i < _n1 * _sz_line_n2; i++)
    {
      num_used += __builtin_popcountl(_used[i]);
    }

  printf ("sp pairs used: %"PRIu64"\n", num_used);

  /* Dump the pairs of sp-states in use. */

  size_t sz_sp_pairs = sizeof (uint32_t) * num_used;

  uint32_t *sp_pairs =
    (uint32_t *) malloc (sz_sp_pairs);

  if (!sp_pairs)
    FATAL("Memory allocation error (sp_pair_use::sp_pairs).");

  /*
  for (unsigned int j1 = 0; j1 < _n1; j1++)
    {
      printf ("%3d:", j1);
      for (size_t i2 = _sz_line_n2; i2; i2--)
	printf (" %016lx", _used[j1 * _sz_line_n2 + i2-1]);
      printf ("\n");
    }
  */

  uint32_t *p = sp_pairs;

  for (unsigned int j1 = 0; j1 < _n1; j1++)
    {
      for (unsigned int j2 = 0; j2 < _n2; j2++)
	{
	  if (used(j1, j2))
	    {
	      *p = (j2 << 16) | j1;
	      p++;
	    }
	}
    }

  assert(p - sp_pairs == (ssize_t) num_used);

  out.fwrite(sp_pairs, sz_sp_pairs, 1);

  free (sp_pairs);

  _num_pairs = num_used;
}
